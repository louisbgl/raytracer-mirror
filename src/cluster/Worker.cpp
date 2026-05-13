#include "Worker.hpp"
#include "../network/TcpClient.hpp"
#include "../network/Message.hpp"
#include "../core/Core.hpp"
#include "../core/Image.hpp"
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
    std::cout << "Connected, waiting for first chunk...\n";

    sock.send(Message::makeReady());

    // Receive first assignment (includes scene content)
    Message msg = sock.receive();
    if (msg.type == MessageType::ABORT) {
        std::cout << "Coordinator rejected connection. Exiting.\n";
        return;
    }
    if (msg.type != MessageType::ASSIGN)
        throw std::runtime_error("Worker: expected ASSIGN, got unexpected message");

    AssignPayload first = msg.parseAssign();

    // Write scene to temp file once — reused for all chunks
    std::string tempPath = "/tmp/raytracer_worker_" + std::to_string(::getpid()) + ".txt";
    {
        std::ofstream tmpFile(tempPath);
        if (!tmpFile.is_open())
            throw std::runtime_error("Worker: cannot create temp scene file");
        tmpFile << first.sceneContent;
    }
    std::cout << "Scene written to " << tempPath << "\n";

    int firstRow = first.firstRow;
    int lastRow  = first.lastRow;
    int width    = first.width;
    int totalRowsRendered = 0;

    // Main render loop — process chunks until FINISH
    while (true) {
        std::cout << "Rendering rows " << firstRow << "-" << lastRow << "...\n";

        std::atomic<bool> done{false};
        std::atomic<bool> renderError{false};
        std::unique_ptr<Image> result;

        std::thread renderThread([&]() {
            try {
                Core core(tempPath);
                core.setProgressTarget(&_progress, nullptr);
                result = std::make_unique<Image>(core.renderSlice(firstRow, lastRow));
            } catch (const std::exception& e) {
                std::cerr << "Worker: render error: " << e.what() << "\n";
                renderError = true;
            }
            done = true;
        });

        int totalRows = lastRow - firstRow;
        while (!done.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (done.load())
                break;
            int percent = totalRows > 0 ? (_progress.load() * 100 / totalRows) : 0;
            sock.send(Message::makeHeartbeat(percent));
        }

        renderThread.join();

        if (renderError) {
            sock.send(Message::makeAbort());
            break;
        }

        // Send pixels in 1000px chunks
        static constexpr int MAX_PIXELS_PER_MSG = 1000;
        int chunkRows  = lastRow - firstRow;
        int totalPixels = chunkRows * width;
        int pixelsSent  = 0;

        for (int y = firstRow; y < lastRow; ++y) {
            int xStart = 0;
            while (xStart < width) {
                int xEnd = std::min(xStart + MAX_PIXELS_PER_MSG, width);
                std::vector<Vec3> pixels;
                pixels.reserve(xEnd - xStart);
                for (int x = xStart; x < xEnd; ++x)
                    pixels.push_back(result->getPixel(x, y - firstRow));
                sock.send(Message::makePixels(pixels));
                pixelsSent += xEnd - xStart;
                xStart = xEnd;
            }

            int rowsSent = y - firstRow + 1;
            if (rowsSent % 5 == 0) {
                int percent = totalPixels > 0 ? (pixelsSent * 100 / totalPixels) : 100;
                sock.send(Message::makeHeartbeat(percent));
            }
        }

        totalRowsRendered += chunkRows;
        std::cout << "Chunk done — total rows rendered: " << totalRowsRendered << "\n";

        // Wait for next chunk or finish
        Message response = sock.receive();

        if (response.type == MessageType::FINISH) {
            std::cout << "All work done. Disconnecting.\n";
            break;
        }
        if (response.type == MessageType::CHUNK) {
            ChunkPayload next = response.parseChunk();
            firstRow = next.firstRow;
            lastRow  = next.lastRow;
            _progress.store(0);
            continue;
        }

        std::cerr << "Worker: unexpected message after chunk, stopping.\n";
        break;
    }

    ::unlink(tempPath.c_str());
}
