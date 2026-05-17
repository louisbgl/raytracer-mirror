#pragma once

#include "TcpSocket.hpp"
#include <memory>
#include <string>

class TcpClient {
public:
    TcpClient(const std::string& host, int port);
    ~TcpClient() = default;

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    TcpSocket& socket() { return *_socket; }

private:
    std::unique_ptr<TcpSocket> _socket;
};
