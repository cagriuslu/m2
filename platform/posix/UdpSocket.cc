#include <m2/network/UdpSocket.h>
#include <m2/Log.h>
#include "PlatformSpecificSocketData.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateServerSideSocket(const Port& port) {
	const auto socketResult = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == -1) {
		return make_unexpected(strerror(errno));
	}

	// Bind the socket to the local port
	sockaddr_in sin{};
#ifdef __APPLE__
	sin.sin_len = sizeof(sin);
#endif
	sin.sin_family = AF_INET;
	sin.sin_port = port.GetInNetworkOrder();
	sin.sin_addr.s_addr = INADDR_ANY;
	if (const auto bindResult = bind(socketResult, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)); bindResult == -1) {
		return make_unexpected(strerror(errno));
	}

	UdpSocket udpSocket;
	udpSocket._selfPort = port;
	udpSocket._platformSpecificData = new detail::PlatformSpecificSocketData{.fd = socketResult};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateClientSideSocket() {
	const auto socketResult = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == -1) {
		return make_unexpected(strerror(errno));
	}

	UdpSocket udpSocket;
	udpSocket._platformSpecificData = new detail::PlatformSpecificSocketData{.fd = socketResult};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateSendOnlyMulticastSocket(const IpAddress& interfaceAddr) {
	const auto socketResult = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == -1) {
		return make_unexpected(strerror(errno));
	}

	// Setting TTL to 1 has a special meaning that restricts the packets to the local network only
	constexpr uint8_t ttl = 1;
	if (const auto ttlResult = setsockopt(socketResult, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)); ttlResult == -1) {
		return make_unexpected(strerror(errno));
	}

	if (interfaceAddr == IpAddress::CreateFromString("127.0.0.1")) {
		LOG_WARN("127.0.0.1 is being used as multicast interface, network discovery may not work on all platforms");
	}

	// Set the interface that the packets will be sent from
	in_addr interfaceInAddr{};
	interfaceInAddr.s_addr = interfaceAddr.GetInNetworkOrder();
	if (const auto interfaceResult = setsockopt(socketResult, IPPROTO_IP, IP_MULTICAST_IF, &interfaceInAddr, sizeof(interfaceInAddr)); interfaceResult == -1) {
		return make_unexpected(strerror(errno));
	}

	// Enable loopback so that clients on this machine can receive the packets
	constexpr uint8_t loop = 1;
	if (const auto loopResult = setsockopt(socketResult, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)); loopResult == -1) {
		return make_unexpected(strerror(errno));
	}

	UdpSocket udpSocket;
	udpSocket._platformSpecificData = new detail::PlatformSpecificSocketData{.fd = socketResult};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateReceiveOnlyMulticastSocket(const IpAddress& groupAddr, const Port& port) {
	const auto socketResult = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == -1) {
		return make_unexpected(strerror(errno));
	}

	// There could be other clients in this machine, allow them to listen for incoming messages
	constexpr int reuse = 1;
	if (const auto reuseResult = setsockopt(socketResult, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); reuseResult == -1) {
		return make_unexpected(strerror(errno));
	}

	// Bind the socket to the local port
	sockaddr_in sin{};
#ifdef __APPLE__
	sin.sin_len = sizeof(sin);
#endif
	sin.sin_family = AF_INET;
	sin.sin_port = port.GetInNetworkOrder();
	sin.sin_addr.s_addr = INADDR_ANY;
	if (const auto bindResult = bind(socketResult, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)); bindResult == -1) {
		return make_unexpected(strerror(errno));
	}

	// Join the multicast group on all interfaces
	ip_mreq mreq{};
	mreq.imr_multiaddr.s_addr = groupAddr.GetInNetworkOrder();
	mreq.imr_interface.s_addr = INADDR_ANY;
	if (const auto membershipResult = ::setsockopt(socketResult, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)); membershipResult == -1) {
		return make_unexpected(strerror(errno));
	}

	UdpSocket udpSocket;
	udpSocket._selfPort = port;
	udpSocket._platformSpecificData = new detail::PlatformSpecificSocketData{.fd = socketResult};
	return std::move(udpSocket);
}
m2::network::UdpSocket::UdpSocket(UdpSocket&& other) noexcept {
	*this = std::move(other);
}
m2::network::UdpSocket& m2::network::UdpSocket::operator=(UdpSocket&& other) noexcept {
	std::swap(_platformSpecificData, other._platformSpecificData);
	std::swap(_selfPort, other._selfPort);
	return *this;
}
m2::network::UdpSocket::~UdpSocket() {
	if (_platformSpecificData) {
		if (0 <= _platformSpecificData->fd) {
			close(_platformSpecificData->fd);
		}
		delete _platformSpecificData;
	}
}

m2::void_expected m2::network::UdpSocket::Send(const IpAddressAndPort& peerAddrAndPort, const uint8_t* buffer, const size_t length) {
	sockaddr_in sin{};
#ifdef __APPLE__
	sin.sin_len = sizeof(sin);
#endif
	sin.sin_family = AF_INET;
	sin.sin_port = peerAddrAndPort.port.GetInNetworkOrder();
	sin.sin_addr.s_addr = peerAddrAndPort.ipAddress.GetInNetworkOrder();

	const auto sendResult = ::sendto(_platformSpecificData->fd, buffer, length, 0, reinterpret_cast<sockaddr*>(&sin), sizeof(sin));
	if (sendResult == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return make_unexpected("Blocked socket returned EAGAIN");
		}
		return make_unexpected(strerror(errno));
	}
	if (sendResult != I(length)) {
		return make_unexpected("Unable to send the whole UDP payload");
	}
	return {};
}

m2::expected<std::pair<int, m2::network::IpAddressAndPort>> m2::network::UdpSocket::Recv(uint8_t* buffer, const size_t length) {
	// Prepare source address
	sockaddr_in sin{};
	socklen_t slen = sizeof(sin);
	const auto recvResult = ::recvfrom(_platformSpecificData->fd, buffer, length, 0, reinterpret_cast<sockaddr*>(&sin), &slen);
	if (recvResult == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			throw M2_ERROR("Blocking socket returned EAGAIN for recv");
		}
		return make_unexpected(strerror(errno));
	}
	if (recvResult == 0) {
		return make_unexpected("Unexpected recvfrom result for a UDP socket: 0");
	}
	return std::make_pair(I(recvResult), IpAddressAndPort{
		.ipAddress = IpAddress::CreateFromNetworkOrder(sin.sin_addr.s_addr),
		.port = Port::CreateFromNetworkOrder(sin.sin_port)
	});
}
