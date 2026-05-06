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

    void close();
    bool isOpen() const { return _fd >= 0; }
    int port() const { return _port; }

private:
    int _fd;
    int _port;
};
