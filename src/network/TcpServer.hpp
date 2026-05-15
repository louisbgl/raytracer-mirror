#pragma once

#include "TcpSocket.hpp"
#include <memory>

class TcpServer {
public:
    explicit TcpServer(int port);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    std::unique_ptr<TcpSocket> accept() const;

    int fd() const { return _fd; }

private:
    int _fd;
};
