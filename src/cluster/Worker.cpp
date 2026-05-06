#include "Worker.hpp"
#include "../network/TcpClient.hpp"
#include "../network/Message.hpp"
#include "../core/Core.hpp"
#include "../core/Image.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

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
    std::cout << "Received chunk: rows " << chunk.firstRow << " to " << chunk.lastRow
              << " of scene " << chunk.scenePath << "\n";

    std::atomic<bool> done{false};
    std::atomic<bool> renderError{false};
    std::unique_ptr<Image> result;

    std::thread renderThread([&]() {
        try {
            Core core(chunk.scenePath);
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

    if (renderError) {
        sock.send(Message::makeAbort());
        return;
    }

    std::cout << "Render done, sending pixels...\n";
    std::vector<Vec3> pixels;
    pixels.reserve(chunk.width * (chunk.lastRow - chunk.firstRow));
    for (int y = chunk.firstRow; y < chunk.lastRow; ++y) {
        for (int x = 0; x < chunk.width; ++x) {
            pixels.push_back(result->getPixel(x, y));
        }
    }
    sock.send(Message::makePixels(pixels));

    Message ack = sock.receive();
    if (ack.type == MessageType::ACK)
        std::cout << "Chunk acknowledged, disconnecting...\n";
    else
        std::cerr << "Worker: unexpected response after PIXELS\n";
}
