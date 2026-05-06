#include "Coordinator.hpp"
#include "../network/Message.hpp"
#include "../core/Core.hpp"
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <stdexcept>
#include <thread>

Coordinator::Coordinator(const std::string& sceneFile)
    : _sceneFile(sceneFile) {}

void Coordinator::run()
{
    _waitForWorkers();
    _distributeWork();

    Image image(_imageWidth, _imageHeight);

    int totalNodes  = static_cast<int>(_workers.size()) + 1;
    int rowsPerNode = _imageHeight / totalNodes;
    int coordFirst  = rowsPerNode * static_cast<int>(_workers.size());

    _monitorAndCollect(image);
    _renderLocalChunk(image, coordFirst, _imageHeight);

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

            _workers.push_back({std::move(socket), ip, 0, 0, 0, false});
            std::cout << "> Worker " << _workers.size() << " connected (" << ip << ")"
                      << " — " << (_workers.size() + 1) << " nodes total (coordinator + "
                      << _workers.size() << " workers)\n";
        }

        if (pfds[1].revents & POLLIN) {
            std::string line;
            std::getline(std::cin, line);
            started = true;
        }
    }

    std::cout << "Starting render with " << (_workers.size() + 1) << " nodes ("
              << _workers.size() << " workers + coordinator)\n";
}

void Coordinator::_distributeWork()
{
    Core core(_sceneFile);
    if (!core.loadScene())
        throw std::runtime_error("Coordinator: failed to load scene");
    _imageWidth  = core.sceneWidth();
    _imageHeight = core.sceneHeight();

    int totalNodes = static_cast<int>(_workers.size()) + 1;
    int rowsPerNode = _imageHeight / totalNodes;

    std::cout << "Dividing " << _imageHeight << " rows across " << totalNodes << " nodes...\n";

    for (int i = 0; i < static_cast<int>(_workers.size()); ++i) {
        int first = i * rowsPerNode;
        int last  = (i + 1) * rowsPerNode;

        _workers[i].firstRow = first;
        _workers[i].lastRow  = last;

        AssignPayload payload{_sceneFile, first, last, _imageWidth, _imageHeight};
        _workers[i].socket->send(Message::makeAssign(payload));

        std::cout << "  > Assigned rows " << first << "-" << last
                  << " to worker " << (i + 1) << " (" << _workers[i].ip << ")\n";
    }

    int coordFirst = rowsPerNode * static_cast<int>(_workers.size());
    int coordLast  = _imageHeight;
    std::cout << "  > Rendering rows " << coordFirst << "-" << coordLast << " locally\n";
}

void Coordinator::_monitorAndCollect(Image& image)
{
    static constexpr int HEARTBEAT_TIMEOUT_MS = 12000; // 2 missed heartbeats at 5s each

    int workerCount = static_cast<int>(_workers.size());
    if (workerCount == 0)
        return;

    std::vector<struct pollfd> pfds(workerCount);
    for (int i = 0; i < workerCount; ++i) {
        pfds[i].fd     = _workers[i].socket->fd();
        pfds[i].events = POLLIN;
    }

    int doneCount = 0;
    while (doneCount < workerCount) {
        int ready = ::poll(pfds.data(), workerCount, HEARTBEAT_TIMEOUT_MS);

        if (ready == 0) {
            // timeout — check missed heartbeats
            for (int i = 0; i < workerCount; ++i) {
                if (_workers[i].done)
                    continue;
                _workers[i].missedHeartbeats++;
                if (_workers[i].missedHeartbeats >= 2) {
                    std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                              << ") presumed dead — reassigning chunk\n";
                    _reassignChunk(image, _workers[i].firstRow, _workers[i].lastRow);
                    _workers[i].done = true;
                    pfds[i].fd = -1; // stop polling this socket
                    doneCount++;
                }
            }
            continue;
        }

        for (int i = 0; i < workerCount; ++i) {
            if (_workers[i].done || !(pfds[i].revents & POLLIN))
                continue;

            Message msg = _workers[i].socket->receive();

            if (msg.type == MessageType::HEARTBEAT) {
                int percent = msg.parseHeartbeat();
                _workers[i].missedHeartbeats = 0;
                std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                          << ") — " << percent << "% done\n";

            } else if (msg.type == MessageType::PIXELS) {
                std::vector<Vec3> pixels = msg.parsePixels();
                int x = 0;
                int y = _workers[i].firstRow;
                for (const Vec3& pixel : pixels) {
                    image.setPixel(x, y, pixel);
                    x++;
                    if (x >= _imageWidth) {
                        x = 0;
                        y++;
                    }
                }
                _workers[i].socket->send(Message::makeAck());
                std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                          << ") — done, pixels received\n";
                _workers[i].done = true;
                pfds[i].fd = -1;
                doneCount++;

            } else if (msg.type == MessageType::ABORT) {
                std::cout << "Worker " << (i + 1) << " (" << _workers[i].ip
                          << ") reported error — reassigning chunk\n";
                _reassignChunk(image, _workers[i].firstRow, _workers[i].lastRow);
                _workers[i].done = true;
                pfds[i].fd = -1;
                doneCount++;
            }
        }
    }
}

void Coordinator::_renderLocalChunk(Image& image, int firstRow, int lastRow)
{
    Core core(_sceneFile);
    Image slice = core.renderSlice(firstRow, lastRow);
    for (int y = firstRow; y < lastRow; ++y) {
        for (int x = 0; x < _imageWidth; ++x) {
            image.setPixel(x, y, slice.getPixel(x, y));
        }
    }
    std::cout << "Local render done\n";
}

void Coordinator::_reassignChunk(Image& image, int firstRow, int lastRow)
{
    std::cout << "Rendering reassigned chunk (rows " << firstRow << "-" << lastRow << ") locally\n";
    _renderLocalChunk(image, firstRow, lastRow);
}
