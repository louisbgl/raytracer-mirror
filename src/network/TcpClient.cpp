#include "TcpClient.hpp"
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

TcpClient::TcpClient(const std::string& host, int port)
{
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("TcpClient: failed to create socket");

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(port));

    if (::inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        ::close(fd);
        throw std::runtime_error("TcpClient: invalid address " + host);
    }

    if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        throw std::runtime_error("TcpClient: cannot connect to " + host + ":" + std::to_string(port));
    }

    _socket = std::make_unique<TcpSocket>(fd);
}
