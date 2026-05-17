#include "TcpServer.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

TcpServer::TcpServer(int port) : _fd(-1)
{
    _fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        throw std::runtime_error("TcpServer: failed to create socket");

    int opt = 1;
    ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port));

    if (::bind(_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
        throw std::runtime_error("TcpServer: failed to bind port " + std::to_string(port));

    if (::listen(_fd, SOMAXCONN) < 0)
        throw std::runtime_error("TcpServer: failed to listen");
}

TcpServer::~TcpServer()
{
    if (_fd >= 0) {
        ::close(_fd);
        _fd = -1;
    }
}

std::unique_ptr<TcpSocket> TcpServer::accept() const
{
    int clientFd = ::accept(_fd, nullptr, nullptr);
    if (clientFd < 0)
        throw std::runtime_error("TcpServer: accept failed");
    return std::make_unique<TcpSocket>(clientFd);
}

