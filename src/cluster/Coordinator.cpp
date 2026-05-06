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
    // phases 2, 3, 4 will follow
}

void Coordinator::_waitForWorkers()
{
    TcpServer server(COORDINATOR_PORT);
    std::cout << "Coordinator started on port " << COORDINATOR_PORT << "\n";
    std::cout << "Waiting for workers... (press Enter to start)\n";

    struct pollfd pfds[2];
    pfds[0].fd     = server.fd();   // watch for new worker connections
    pfds[0].events = POLLIN;
    pfds[1].fd     = STDIN_FILENO;  // watch for Enter key
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
