#include <m2/network/UdpSocket.h>
#include <m2/Log.h>
#include "PlatformSpecificUdpSocketData.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

m2::expected<m2::UdpSocket> m2::UdpSocket::Create(const network::Port localPort) {
	const auto socketResult = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == -1) {
		return make_unexpected(strerror(errno));
	}

	// Bind the socket to the local port
	sockaddr_in sin{};
#ifdef __APPLE__
	sin.sin_len = sizeof(sin);
#endif
	sin.sin_family = AF_INET;
	sin.sin_port = localPort;
	sin.sin_addr.s_addr = INADDR_ANY;
	if (const auto bindResult = ::bind(socketResult, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)); bindResult == -1) {
		return make_unexpected(strerror(errno));
	}

	UdpSocket udpSocket;
	udpSocket._platformSpecificData = new detail::PlatformSpecificUdpSocketData{.fd = socketResult};
	return std::move(udpSocket);
}
m2::UdpSocket::UdpSocket(UdpSocket&& other) noexcept {
	*this = std::move(other);
}
m2::UdpSocket& m2::UdpSocket::operator=(UdpSocket&& other) noexcept {
	std::swap(_platformSpecificData, other._platformSpecificData);
	return *this;
}
m2::UdpSocket::~UdpSocket() {
	if (_platformSpecificData) {
		if (0 <= _platformSpecificData->fd) {
			close(_platformSpecificData->fd);
		}
		delete _platformSpecificData;
	}
}

m2::expected<int> m2::UdpSocket::send(const network::IpAddress peerAddr, const network::Port peerPort, const uint8_t* buffer, const size_t length) {
	// Prepare destination address
	sockaddr_in sin{};
#ifdef __APPLE__
	sin.sin_len = sizeof(sin);
#endif
	sin.sin_family = AF_INET;
	sin.sin_port = peerPort;
	sin.sin_addr.s_addr = peerAddr;

	const auto sendResult = ::sendto(_platformSpecificData->fd, buffer, length, 0, reinterpret_cast<sockaddr*>(&sin), sizeof(sin));
	if (sendResult == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return -1;
		}
		return make_unexpected(strerror(errno));
	}
	return I(sendResult);
}

m2::expected<std::pair<int, m2::network::IpAddressAndPort>> m2::UdpSocket::recv(uint8_t* buffer, const size_t length) {
	// Prepare source address
	sockaddr_in sin{};
	socklen_t slen = sizeof(sin);
	const auto recvResult = ::recvfrom(_platformSpecificData->fd, buffer, length, 0, reinterpret_cast<sockaddr*>(&sin), &slen);
	if (recvResult == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return std::make_pair<int, network::IpAddressAndPort>(0, {});
		}
		return make_unexpected(strerror(errno));
	}
	if (recvResult == 0) {
		return make_unexpected("Unexpected recvfrom result for a UDP socket: 0");
	}
	return std::make_pair(I(recvResult), network::IpAddressAndPort{sin.sin_addr.s_addr, sin.sin_port});
}
