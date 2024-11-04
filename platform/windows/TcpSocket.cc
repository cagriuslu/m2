#include <m2/network/TcpSocket.h>

using namespace m2;
using namespace m2::network;

TcpSocket::TcpSocket(int fd) {}

TcpSocket::TcpSocket(int fd, IpAddress addr, Port port) {}

expected<TcpSocket> TcpSocket::create() {
    return TcpSocket{0};
}

TcpSocket::TcpSocket(TcpSocket&& other) noexcept {}

TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept { return *this; }

TcpSocket::~TcpSocket() {}

expected<bool> TcpSocket::bind(uint16_t port) {
    return {};
}

void_expected TcpSocket::listen(int queue_size) {
    return {};
}

expected<bool> TcpSocket::connect(const std::string& ip_addr, uint16_t port) {
    return {};
}

expected<std::optional<TcpSocket>> TcpSocket::accept() {
    return {};
}

expected<int> TcpSocket::send(const uint8_t* buffer, size_t length) {
    return {};
}

expected<int> TcpSocket::recv(uint8_t* buffer, size_t length) {
    return {};
}
