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
    std::cout << "Connected, waiting for chunk...\n";

    sock.send(Message::makeReady());

    Message msg = sock.receive();
    if (msg.type == MessageType::ABORT) {
        std::cout << "Coordinator rejected connection. Exiting.\n";
        return;
    }
    if (msg.type != MessageType::ASSIGN)
        throw std::runtime_error("Worker: unexpected message, expected ASSIGN");

    AssignPayload chunk = msg.parseAssign();
    std::cout << "Received chunk: rows " << chunk.firstRow << " to " << chunk.lastRow << "\n";

    // write scene content to a temp file
    std::string tempPath = "/tmp/raytracer_worker_" + std::to_string(::getpid()) + ".txt";
    {
        std::ofstream tmpFile(tempPath);
        if (!tmpFile.is_open())
            throw std::runtime_error("Worker: cannot create temp scene file");
        tmpFile << chunk.sceneContent;
    }
    std::cout << "Scene written to " << tempPath << "\n";

    std::atomic<bool> done{false};
    std::atomic<bool> renderError{false};
    std::unique_ptr<Image> result;

    std::thread renderThread([&]() {
        try {
            Core core(tempPath);
            core.setProgressTarget(&_progress, nullptr);
            result = std::make_unique<Image>(core.renderSlice(chunk.firstRow, chunk.lastRow));
        } catch (const std::exception& e) {
            std::cerr << "Worker: render error: " << e.what() << "\n";
            renderError = true;
        }
        done = true;
    });

    int totalRows = chunk.lastRow - chunk.firstRow;
    while (!done.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (done.load())
            break;
        int percent = totalRows > 0 ? (_progress.load() * 100 / totalRows) : 0;
        sock.send(Message::makeHeartbeat(percent));
    }

    renderThread.join();
    ::unlink(tempPath.c_str());

    if (renderError) {
        sock.send(Message::makeAbort());
        return;
    }

    // send pixels row by row, sending a heartbeat every 100 rows to stay alive
    std::cout << "Render done, sending pixels...\n";
    int totalRows = chunk.lastRow - chunk.firstRow;
    for (int y = chunk.firstRow; y < chunk.lastRow; ++y) {
        std::vector<Vec3> row;
        row.reserve(chunk.width);
        for (int x = 0; x < chunk.width; ++x) {
            row.push_back(result->getPixel(x, y));
        }
        sock.send(Message::makePixels(row));

        int rowsSent = y - chunk.firstRow + 1;
        if (rowsSent % 100 == 0) {
            int percent = totalRows > 0 ? (rowsSent * 100 / totalRows) : 100;
            sock.send(Message::makeHeartbeat(percent));
        }
    }

    Message ack = sock.receive();
    if (ack.type == MessageType::ACK)
        std::cout << "Chunk acknowledged, disconnecting...\n";
    else
        std::cerr << "Worker: unexpected response after PIXELS\n";
}
