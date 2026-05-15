#include "Coordinator.hpp"
#include "../network/Message.hpp"
#include "../core/Core.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <poll.h>
#include <unistd.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <atomic>

Coordinator::Coordinator(const std::string& sceneFile)
    : _sceneFile(sceneFile) {}

void Coordinator::run()
{
    std::cout << "Log: " << _log.path() << "\n";
    _log.info("coordinator", "Coordinator started on port " + std::to_string(COORDINATOR_PORT));

    _waitForWorkers();
    _distributeWork();

    Image image(_imageWidth, _imageHeight);

    _renderStart = std::chrono::steady_clock::now();

    std::thread localThread([&]() { _renderLocalChunks(image); });

    std::atomic<bool> renderDone{false};
    std::thread dashThread([&]() {
        while (!renderDone.load()) {
            _drawDashboard();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    try {
        _monitorAndCollect(image);
    } catch (const std::exception& e) {
        _log.error("coordinator", std::string("Monitor error: ") + e.what());
    }

    localThread.join();
    renderDone = true;
    dashThread.join();
    _drawDashboard();
    auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - _renderStart).count();
    int mins = static_cast<int>(elapsed) / 60;
    double secs = elapsed - mins * 60;

    std::cout << "Render time: ";
    if (mins > 0)
        std::cout << mins << "m ";
    std::cout << std::fixed << std::setprecision(2) << secs << "s\n";

    std::ostringstream timeStr;
    if (mins > 0) timeStr << mins << "m ";
    timeStr << std::fixed << std::setprecision(2) << secs << "s";
    _log.info("coordinator", "Render time: " + timeStr.str());

    image.writeFile(_outputFile);
    _log.info("coordinator", "Image saved to " + _outputFile);
}

void Coordinator::_waitForWorkers()
{
    TcpServer server(COORDINATOR_PORT);
    std::cout << "Coordinator started on port " << COORDINATOR_PORT << "\n";
    std::cout << "Waiting for workers... (press Enter to start)\n";

    struct pollfd pfds[2];
    pfds[0].fd     = server.fd();
    pfds[0].events = POLLIN;
    pfds[1].fd     = STDIN_FILENO;
    pfds[1].events = POLLIN;

    bool started = false;
    while (!started) {
        int ready = ::poll(pfds, 2, -1);
        if (ready <= 0)
            continue;

        if (pfds[0].revents & POLLIN) {
            auto socket = server.accept();
            std::string ip = socket->peerAddress();

            Message msg = socket->receive();
            if (msg.type != MessageType::READY) {
                std::cout << "Rejected connection from " << ip << " (unexpected message)\n";
                continue;
            }

            WorkerInfo w;
            w.socket = std::move(socket);
            w.ip     = ip;
            _workers.push_back(std::move(w));
            std::cout << "> Worker " << _workers.size() << " connected (" << ip << ")"
                      << " — " << _workers.size() << " worker(s) total\n";
            _log.info(ip, "Worker connected (" + std::to_string(_workers.size()) + " total)");
        }

        if (pfds[1].revents & POLLIN) {
            std::string line;
            std::getline(std::cin, line);
            started = true;
        }
    }

    std::cout << "Starting render with " << _workers.size() << " worker(s)\n";
    _log.info("coordinator", "Starting render with " + std::to_string(_workers.size()) + " worker(s)");
}

void Coordinator::_distributeWork()
{
    Core core(_sceneFile);
    if (!core.loadScene())
        throw std::runtime_error("Coordinator: failed to load scene");
    _imageWidth  = core.sceneWidth();
    _imageHeight = core.sceneHeight();
    _outputFile  = core.outputFile();

    std::ifstream file(_sceneFile);
    if (!file.is_open())
        throw std::runtime_error("Coordinator: cannot read scene file " + _sceneFile);
    std::ostringstream ss;
    ss << file.rdbuf();
    _sceneContent = ss.str();

    // Build queue of CHUNK_SIZE-row chunks
    for (int row = 0; row < _imageHeight; row += CHUNK_SIZE) {
        int last = std::min(row + CHUNK_SIZE, _imageHeight);
        _pendingChunks.push({row, last});
    }

    _totalChunks = static_cast<int>(_pendingChunks.size());
    std::cout << "Image: " << _imageWidth << "x" << _imageHeight
              << " — " << _totalChunks << " chunks of " << CHUNK_SIZE << " rows\n";
    std::cout << "Assigning first chunks to " << _workers.size() << " worker(s)...\n";

    for (int i = 0; i < static_cast<int>(_workers.size()); ++i) {
        if (!_pendingChunks.empty()) {
            _assignNextChunk(i);
        } else {
            // No chunks left — send FINISH immediately so this worker doesn't hang
            _workers[i].socket->send(Message::makeFinish());
            _workers[i].done = true;
            _log.info(_workers[i].ip, "No chunks available, sent FINISH immediately");
        }
    }
}

void Coordinator::_assignNextChunk(int i)
{
    std::lock_guard<std::mutex> lock(_chunksMutex);
    auto [first, last] = _pendingChunks.front();
    _pendingChunks.pop();

    _workers[i].firstRow       = first;
    _workers[i].lastRow        = last;
    _workers[i].pixelsReceived = 0;

    if (_workers[i].totalRowsRendered == 0) {
        _workers[i].startTime = std::chrono::steady_clock::now();
        AssignPayload payload{_sceneContent, first, last, _imageWidth, _imageHeight};
        _workers[i].socket->send(Message::makeAssign(payload));
    } else {
        ChunkPayload payload{first, last};
        _workers[i].socket->send(Message::makeChunk(payload));
    }

}

void Coordinator::_monitorAndCollect(Image& image)
{
    static constexpr int HEARTBEAT_TIMEOUT_MS = 12000;

    int workerCount = static_cast<int>(_workers.size());
    if (workerCount == 0)
        return;

    std::vector<struct pollfd> pfds(workerCount);
    int doneCount = 0;
    for (int i = 0; i < workerCount; ++i) {
        if (_workers[i].done) {
            pfds[i].fd = -1;
            doneCount++;
        } else {
            pfds[i].fd     = _workers[i].socket->fd();
            pfds[i].events = POLLIN;
        }
    }

    while (doneCount < workerCount) {
        int ready = ::poll(pfds.data(), workerCount, HEARTBEAT_TIMEOUT_MS);

        if (ready == 0) {
            for (int i = 0; i < workerCount; ++i) {
                if (_workers[i].done)
                    continue;
                _workers[i].missedHeartbeats++;
                if (_workers[i].missedHeartbeats >= 2) {
                    _log.warn(_workers[i].ip, "Timed out — re-queuing rows "
                        + std::to_string(_workers[i].firstRow) + "-" + std::to_string(_workers[i].lastRow));
                    { std::lock_guard<std::mutex> lock(_chunksMutex); _pendingChunks.push({_workers[i].firstRow, _workers[i].lastRow}); }
                    _workers[i].done = true;
                    pfds[i].fd = -1;
                    doneCount++;
                }
            }
            continue;
        }

        for (int i = 0; i < workerCount; ++i) {
            if (_workers[i].done || !(pfds[i].revents & POLLIN))
                continue;

            Message msg;
            try {
                msg = _workers[i].socket->receive();
            } catch (const std::exception& e) {
                _log.error(_workers[i].ip, std::string("Connection lost: ") + e.what()
                    + " — re-queuing rows " + std::to_string(_workers[i].firstRow)
                    + "-" + std::to_string(_workers[i].lastRow));
                { std::lock_guard<std::mutex> lock(_chunksMutex); _pendingChunks.push({_workers[i].firstRow, _workers[i].lastRow}); }
                _workers[i].done = true;
                pfds[i].fd = -1;
                doneCount++;
                continue;
            }

            if (msg.type == MessageType::PIXELS) {
                _workers[i].missedHeartbeats = 0;
                std::vector<Vec3> chunk = msg.parsePixels();

                for (int k = 0; k < static_cast<int>(chunk.size()); ++k) {
                    int pixelIdx = _workers[i].pixelsReceived + k;
                    int y = _workers[i].firstRow + pixelIdx / _imageWidth;
                    int x = pixelIdx % _imageWidth;
                    if (y < _workers[i].lastRow)
                        image.setPixel(x, y, chunk[k]);
                }
                _workers[i].pixelsReceived += static_cast<int>(chunk.size());

                int totalChunkPixels = _imageWidth * (_workers[i].lastRow - _workers[i].firstRow);
                if (_workers[i].pixelsReceived >= totalChunkPixels) {
                    int rows = _workers[i].lastRow - _workers[i].firstRow;
                    {
                        std::lock_guard<std::mutex> lock(_statsMutex);
                        _workers[i].totalRowsRendered += rows;
                    }
                    _chunksCompleted++;

                    bool hasMore;
                    {
                        std::lock_guard<std::mutex> lock(_chunksMutex);
                        hasMore = !_pendingChunks.empty();
                    }
                    if (hasMore) {
                        _assignNextChunk(i);
                    } else {
                        _workers[i].socket->send(Message::makeFinish());
                        _workers[i].done = true;
                        pfds[i].fd = -1;
                        doneCount++;
                        _log.info(_workers[i].ip, "All work done — total rows: "
                            + std::to_string(_workers[i].totalRowsRendered));
                    }
                }

            } else if (msg.type == MessageType::ABORT) {
                _log.error(_workers[i].ip, "Render error — re-queuing rows "
                    + std::to_string(_workers[i].firstRow) + "-"
                    + std::to_string(_workers[i].lastRow));
                { std::lock_guard<std::mutex> lock(_chunksMutex); _pendingChunks.push({_workers[i].firstRow, _workers[i].lastRow}); }
                _workers[i].done = true;
                pfds[i].fd = -1;
                doneCount++;
            }
        }
    }

    _log.info("coordinator", "=== Render complete — " + std::to_string(_workers.size()) + " worker(s) ===");
}

void Coordinator::_renderLocalChunks(Image& image)
{
    while (true) {
        int first = 0, last = 0;
        {
            std::lock_guard<std::mutex> lock(_chunksMutex);
            if (_pendingChunks.empty())
                break;
            auto [f, l] = _pendingChunks.front();
            _pendingChunks.pop();
            first = f;
            last  = l;
        }

        try {
            Core core(_sceneFile);
            Image slice = core.renderSlice(first, last);
            for (int y = first; y < last; ++y)
                for (int x = 0; x < _imageWidth; ++x)
                    image.setPixel(x, y, slice.getPixel(x, y - first));
        } catch (const std::exception& e) {
            _log.error("local", std::string("Render error: ") + e.what());
            std::lock_guard<std::mutex> lock(_chunksMutex);
            _pendingChunks.push({first, last});
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(_statsMutex);
            _coordinatorRowsRendered += (last - first);
        }
        _chunksCompleted++;
    }
}

void Coordinator::_drawDashboard() const
{
    using namespace std::chrono;
    auto now     = steady_clock::now();
    double elapsed = duration<double>(now - _renderStart).count();
    int mins   = static_cast<int>(elapsed) / 60;
    double secs = elapsed - mins * 60;

    int completed = _chunksCompleted.load();
    int total     = _totalChunks;
    double pct    = total > 0 ? (double)completed / total : 0.0;

    // Build progress bar (30 chars wide)
    static constexpr int BAR_WIDTH = 30;
    int filled = static_cast<int>(pct * BAR_WIDTH);
    std::string bar;
    bar.reserve(BAR_WIDTH * 3); // UTF-8: each block char is 3 bytes
    for (int i = 0; i < BAR_WIDTH; ++i)
        bar += (i < filled) ? "█" : "░";

    // Build sorted leaderboard entries
    struct Entry { std::string ip; int rows; double rps; };
    std::vector<Entry> entries;
    {
        std::lock_guard<std::mutex> lock(_statsMutex);
        for (const auto& w : _workers) {
            double e = duration<double>(now - w.startTime).count();
            double rps = (e > 0 && w.totalRowsRendered > 0) ? w.totalRowsRendered / e : 0.0;
            entries.push_back({w.ip, w.totalRowsRendered, rps});
        }
    }
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.rps > b.rps;
    });

    // Clear screen and redraw from top
    std::cout << "\033[2J\033[H";
    std::cout << "┌──────────────────────────────────────────────────────┐\n";
    std::cout << "│           RAYTRACER CLUSTER RENDER                   │\n";
    std::cout << "├──────────────────────────────────────────────────────┤\n";
    std::cout << "│  [" << bar << "] "
              << std::setw(3) << static_cast<int>(pct * 100) << "%"
              << "  (" << completed << "/" << total << " chunks)"
              << std::string(3, ' ') << "│\n";
    std::cout << "├──────┬──────────────────┬──────────┬─────────────────┤\n";
    std::cout << "│ Rank │ Worker IP        │   Rows   │    Rows/sec     │\n";
    std::cout << "├──────┼──────────────────┼──────────┼─────────────────┤\n";
    // Add coordinator as a regular entry
    {
        double localElapsed = duration<double>(now - _renderStart).count();
        double localRps = (localElapsed > 0 && _coordinatorRowsRendered.load() > 0)
                          ? _coordinatorRowsRendered.load() / localElapsed : 0.0;
        entries.push_back({"local", _coordinatorRowsRendered.load(), localRps});
        std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
            return a.rps > b.rps;
        });
    }
    for (int r = 0; r < static_cast<int>(entries.size()); ++r) {
        const auto& e = entries[r];
        std::cout << "│  " << std::setw(3) << (r + 1)
                  << " │ " << std::setw(16) << std::left  << e.ip
                  << " │ " << std::setw(8)  << std::right << e.rows
                  << " │ " << std::setw(13) << std::fixed << std::setprecision(1) << e.rps
                  << "   │\n";
    }
    std::cout << "└──────┴──────────────────┴──────────┴─────────────────┘\n";
    std::cout << "  Elapsed: ";
    if (mins > 0) std::cout << mins << "m ";
    std::cout << std::fixed << std::setprecision(1) << secs << "s"
              << "    Log: " << _log.path() << "\n";
    std::cout.flush();
}
