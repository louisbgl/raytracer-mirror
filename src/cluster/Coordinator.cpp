#include "Coordinator.hpp"
#include "../network/Message.hpp"
#include "../core/Core.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
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
    _waitForWorkers();
    _distributeWork();

    Image image(_imageWidth, _imageHeight);

    try {
        _monitorAndCollect(image);
    } catch (const std::exception& e) {
        std::cerr << "Monitor error: " << e.what() << "\n";
    }

    std::cout << "Assembling final image...\n";
    image.writeFile("output.ppm");
    std::cout << "Image saved to output.ppm\n";
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

            _workers.push_back({std::move(socket), ip, 0, 0, 0, 0, 0, false});
            std::cout << "> Worker " << _workers.size() << " connected (" << ip << ")"
                      << " — " << _workers.size() << " worker(s) total\n";
        }

        if (pfds[1].revents & POLLIN) {
            std::string line;
            std::getline(std::cin, line);
            started = true;
        }
    }

    std::cout << "Starting render with " << _workers.size() << " worker(s)\n";
}

void Coordinator::_distributeWork()
{
    Core core(_sceneFile);
    if (!core.loadScene())
        throw std::runtime_error("Coordinator: failed to load scene");
    _imageWidth  = core.sceneWidth();
    _imageHeight = core.sceneHeight();

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

    int totalChunks = static_cast<int>(_pendingChunks.size());
    std::cout << "Image: " << _imageWidth << "x" << _imageHeight
              << " — " << totalChunks << " chunks of " << CHUNK_SIZE << " rows\n";
    std::cout << "Assigning first chunks to " << _workers.size() << " worker(s)...\n";

    for (int i = 0; i < static_cast<int>(_workers.size()); ++i) {
        if (_pendingChunks.empty())
            break;
        _assignNextChunk(i);
    }
}

void Coordinator::_assignNextChunk(int i)
{
    auto [first, last] = _pendingChunks.front();
    _pendingChunks.pop();

    _workers[i].firstRow       = first;
    _workers[i].lastRow        = last;
    _workers[i].pixelsReceived = 0;

    if (_workers[i].totalRowsRendered == 0) {
        // First assignment — send full scene content
        AssignPayload payload{_sceneContent, first, last, _imageWidth, _imageHeight};
        _workers[i].socket->send(Message::makeAssign(payload));
    } else {
        // Subsequent assignment — send only coords
        ChunkPayload payload{first, last};
        _workers[i].socket->send(Message::makeChunk(payload));
    }

    std::cout << "  > Worker " << (i + 1) << " (" << _workers[i].ip
              << ") assigned rows " << first << "-" << last << "\n";
}

void Coordinator::_monitorAndCollect(Image& image)
{
    static constexpr int HEARTBEAT_TIMEOUT_MS = 12000;

    int workerCount = static_cast<int>(_workers.size());
    if (workerCount == 0)
        return;

    std::vector<struct pollfd> pfds(workerCount);
    for (int i = 0; i < workerCount; ++i) {
        pfds[i].fd     = _workers[i].socket->fd();
        pfds[i].events = POLLIN;
    }

    int doneCount = 0;
    int chunksCompleted = 0;
    int totalChunks = static_cast<int>(
        (_imageHeight + CHUNK_SIZE - 1) / CHUNK_SIZE
    );

    while (doneCount < workerCount) {
        int ready = ::poll(pfds.data(), workerCount, HEARTBEAT_TIMEOUT_MS);

        if (ready == 0) {
            for (int i = 0; i < workerCount; ++i) {
                if (_workers[i].done)
                    continue;
                _workers[i].missedHeartbeats++;
                if (_workers[i].missedHeartbeats >= 2) {
                    std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                              << ") timed out — re-queuing rows "
                              << _workers[i].firstRow << "-" << _workers[i].lastRow << "\n";
                    _pendingChunks.push({_workers[i].firstRow, _workers[i].lastRow});
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
                std::cerr << "Worker " << (i + 1) << " (" << _workers[i].ip
                          << ") connection lost: " << e.what() << "\n";
                _pendingChunks.push({_workers[i].firstRow, _workers[i].lastRow});
                _workers[i].done = true;
                pfds[i].fd = -1;
                doneCount++;
                continue;
            }

            if (msg.type == MessageType::HEARTBEAT) {
                int percent = msg.parseHeartbeat();
                _workers[i].missedHeartbeats = 0;
                std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                          << ") — chunk " << percent << "% done\n";

            } else if (msg.type == MessageType::PIXELS) {
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
                    _workers[i].totalRowsRendered += rows;
                    chunksCompleted++;

                    std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                              << ") — chunk done ["
                              << chunksCompleted << "/" << totalChunks << "]\n";

                    if (!_pendingChunks.empty()) {
                        _assignNextChunk(i);
                    } else {
                        _workers[i].socket->send(Message::makeFinish());
                        _workers[i].done = true;
                        pfds[i].fd = -1;
                        doneCount++;
                        std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                                  << ") finished — total rows rendered: "
                                  << _workers[i].totalRowsRendered << "\n";
                    }
                }

            } else if (msg.type == MessageType::ABORT) {
                std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                          << ") reported error — re-queuing chunk\n";
                _pendingChunks.push({_workers[i].firstRow, _workers[i].lastRow});
                _workers[i].done = true;
                pfds[i].fd = -1;
                doneCount++;
            }
        }
    }

    // Print final summary
    std::cout << "\n=== Render complete ===\n";
    for (int i = 0; i < workerCount; ++i) {
        std::cout << "  Worker " << (i + 1) << " (" << _workers[i].ip
                  << "): " << _workers[i].totalRowsRendered << " rows\n";
    }
}
