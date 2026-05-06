#pragma once

#include "Message.hpp"
#include <string>

class TcpSocket {
public:
    explicit TcpSocket(int fd);
    ~TcpSocket();

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    void send(const Message& message) const;
    Message receive() const;
    void close();

    bool isOpen() const { return _fd >= 0; }
    int fd() const { return _fd; }
    std::string peerAddress() const;

private:
    int _fd;

    void _sendAll(const uint8_t* data, int length) const;
    void _recvAll(uint8_t* data, int length) const;
};
