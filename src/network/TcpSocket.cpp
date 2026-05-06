#include "TcpSocket.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

TcpSocket::TcpSocket(int fd) : _fd(fd) {}

TcpSocket::~TcpSocket()
{
    close();
}

void TcpSocket::close()
{
    if (_fd >= 0) {
        ::close(_fd);
        _fd = -1;
    }
}

void TcpSocket::_sendAll(const uint8_t* data, int length) const
{
    int sent = 0;
    while (sent < length) {
        int n = ::send(_fd, data + sent, length - sent, 0);
        if (n <= 0)
            throw std::runtime_error("TcpSocket: connection lost while sending");
        sent += n;
    }
}

void TcpSocket::_recvAll(uint8_t* data, int length) const
{
    int received = 0;
    while (received < length) {
        int n = ::recv(_fd, data + received, length - received, 0);
        if (n <= 0)
            throw std::runtime_error("TcpSocket: connection lost while receiving");
        received += n;
    }
}

void TcpSocket::send(const Message& message) const
{
    std::vector<uint8_t> bytes = message.serialize();
    _sendAll(bytes.data(), static_cast<int>(bytes.size()));
}

Message TcpSocket::receive() const
{
    // read header: 1 byte type + 4 bytes length
    uint8_t header[5];
    _recvAll(header, 5);

    int payloadLength = (static_cast<int>(header[1]) << 24)
                      | (static_cast<int>(header[2]) << 16)
                      | (static_cast<int>(header[3]) <<  8)
                      | (static_cast<int>(header[4]) <<  0);

    // read payload
    std::vector<uint8_t> full(5 + payloadLength);
    full[0] = header[0];
    full[1] = header[1];
    full[2] = header[2];
    full[3] = header[3];
    full[4] = header[4];

    if (payloadLength > 0)
        _recvAll(full.data() + 5, payloadLength);

    return Message::deserialize(full);
}

std::string TcpSocket::peerAddress() const
{
    struct sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (getpeername(_fd, reinterpret_cast<struct sockaddr*>(&addr), &len) != 0)
        return "unknown";
    return std::string(inet_ntoa(addr.sin_addr));
}
