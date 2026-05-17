#include "Worker.hpp"
#include "../network/TcpClient.hpp"
#include "../network/Message.hpp"
#include "../core/Core.hpp"
#include "../core/Image.hpp"
#include "../core/PluginManager.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <unistd.h>

Worker::Worker(const std::string& host, int port)
    : _host(host), _port(port) {}

void Worker::run()
{
    std::cout << "Connecting to coordinator at " << _host << ":" << _port << "...\n";
    TcpClient client(_host, _port);
    TcpSocket& sock = client.socket();

    // Only create the log file after a successful connection
    std::cout << "Connected. Log: " << _log.path() << "\n";
    _log.info("worker", "Connected to " + _host + ":" + std::to_string(_port));

    sock.send(Message::makeReady());

    Message msg = sock.receive();
    if (msg.type == MessageType::ABORT) {
        std::cout << "Coordinator rejected connection. Exiting.\n";
        return;
    }
    if (msg.type != MessageType::ASSIGN)
        throw std::runtime_error("Worker: expected ASSIGN, got unexpected message");

    AssignPayload first = msg.parseAssign();

    std::string tempPath = "/tmp/raytracer_worker_" + std::to_string(::getpid()) + ".txt";
    {
        std::ofstream tmpFile(tempPath);
        if (!tmpFile.is_open())
            throw std::runtime_error("Worker: cannot create temp scene file");
        tmpFile << first.sceneContent;
    }

    int firstRow = first.firstRow;
    int lastRow  = first.lastRow;
    int width    = first.width;

    std::atomic<bool> workerDone{false};
    std::thread logThread([&]() {
        auto last = std::chrono::steady_clock::now();
        while (!workerDone.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last).count() >= 5) {
                _log.info("worker", "Rows rendered: " + std::to_string(_totalRowsRendered.load()));
                last = now;
            }
        }
    });

    while (true) {
        _log.info("worker", "Rendering rows " + std::to_string(firstRow) + "-" + std::to_string(lastRow));
        bool renderError = false;
        std::unique_ptr<Image> result;

        try {
            Core core(tempPath);
            result = std::make_unique<Image>(core.renderSlice(firstRow, lastRow));
        } catch (const std::exception& e) {
            _log.error("worker", std::string("Render error: ") + e.what());
            renderError = true;
        }

        if (renderError) {
            sock.send(Message::makeAbort());
            break;
        }

        static constexpr int MAX_PIXELS_PER_MSG = 1000;
        int chunkRows = lastRow - firstRow;

        for (int y = firstRow; y < lastRow; ++y) {
            int xStart = 0;
            while (xStart < width) {
                int xEnd = std::min(xStart + MAX_PIXELS_PER_MSG, width);
                std::vector<Vec3> pixels;
                pixels.reserve(xEnd - xStart);
                for (int x = xStart; x < xEnd; ++x)
                    pixels.push_back(result->getPixel(x, y - firstRow));
                sock.send(Message::makePixels(pixels));
                xStart = xEnd;
            }
        }

        _totalRowsRendered += chunkRows;

        Message response = sock.receive();
        if (response.type == MessageType::FINISH)
            break;
        if (response.type == MessageType::CHUNK) {
            ChunkPayload next = response.parseChunk();
            firstRow = next.firstRow;
            lastRow  = next.lastRow;
            continue;
        }
        break;
    }

    workerDone = true;
    logThread.join();
    ::unlink(tempPath.c_str());
    _log.info("worker", "Done — total rows rendered: " + std::to_string(_totalRowsRendered.load()));
    std::cout << "Done. Total rows rendered: " << _totalRowsRendered.load() << "\n";
}

int Worker::launch(const std::string& addr) {
    PluginManager::instance().initialize();
    size_t colon = addr.rfind(':');
    if (colon == std::string::npos) {
        std::cerr << "Error: --worker expects ip:port format\n";
        return 84;
    }
    try {
        std::string host = addr.substr(0, colon);
        int port = std::stoi(addr.substr(colon + 1));
        if (port <= 0 || port > 65535) {
            std::cerr << "Error: port must be between 1 and 65535\n";
            return 84;
        }
        Worker worker(host, port);
        worker.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Worker error: " << e.what() << "\n";
        return 84;
    }
}
