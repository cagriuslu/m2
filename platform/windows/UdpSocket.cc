#include <m2/network/UdpSocket.h>
#include <m2/Log.h>
#include "PlatformSpecificSocketData.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mutex>
#include <format>

#pragma comment(lib, "Ws2_32.lib")

using namespace m2;
using namespace m2::network;

namespace {
	void EnsureWinsockInitialized() {
		static std::once_flag flag;
		static WSADATA wsaData;
		std::call_once(flag, []() {
			if (const int result = WSAStartup(MAKEWORD(2, 2), &wsaData); result != 0) {
				throw M2_ERROR(std::format("WSAStartup failed: {}", result));
			}
		});
	}
}

m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateServerSideSocket(const Port& port) {
	EnsureWinsockInitialized();
	const auto socketResult = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == INVALID_SOCKET) {
		return make_unexpected(std::format("socket failed: {}", WSAGetLastError()));
	}

	// Bind the socket to the local port
	sockaddr_in sin{};
	sin.sin_family = AF_INET;
	sin.sin_port = port.GetInNetworkOrder();
	sin.sin_addr.s_addr = INADDR_ANY;
	if (::bind(socketResult, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) == SOCKET_ERROR) {
		return make_unexpected(std::format("bind failed: {}", WSAGetLastError()));
	}

	UdpSocket udpSocket;
	udpSocket._selfPort = port;
	udpSocket._platformSpecificData = new detail::PlatformSpecificSocketData{.socket = socketResult};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateClientSideSocket() {
	EnsureWinsockInitialized();
	const auto socketResult = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == INVALID_SOCKET) {
		return make_unexpected(std::format("socket failed: {}", WSAGetLastError()));
	}

	UdpSocket udpSocket;
	udpSocket._platformSpecificData = new detail::PlatformSpecificSocketData{.socket = socketResult};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateSendOnlyMulticastSocket(const IpAddress& interfaceAddr) {
	EnsureWinsockInitialized();
	const auto socketResult = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == INVALID_SOCKET) {
		return make_unexpected(std::format("socket failed: {}", WSAGetLastError()));
	}

	// Setting TTL to 1 has a special meaning that restricts the packets to the local network only
	constexpr DWORD ttl = 1;
	if (::setsockopt(socketResult, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<const char*>(&ttl), sizeof(ttl)) == SOCKET_ERROR) {
		return make_unexpected(std::format("setsockopt(IP_MULTICAST_TTL) failed: {}", WSAGetLastError()));
	}

	if (interfaceAddr == IpAddress::CreateLocalhost()) {
		LOG_INFO("127.0.0.1 is being used as multicast interface, network discovery may not work on all platforms");
	}

	// Set the interface that the packets will be sent from
	in_addr interfaceInAddr{};
	interfaceInAddr.s_addr = interfaceAddr.GetInNetworkOrder();
	if (::setsockopt(socketResult, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<const char*>(&interfaceInAddr), sizeof(interfaceInAddr)) == SOCKET_ERROR) {
		return make_unexpected(std::format("setsockopt(IP_MULTICAST_IF) failed: {}", WSAGetLastError()));
	}

	// Enable loopback so that clients on this machine can receive the packets
	constexpr DWORD loop = 1;
	if (::setsockopt(socketResult, IPPROTO_IP, IP_MULTICAST_LOOP, reinterpret_cast<const char*>(&loop), sizeof(loop)) == SOCKET_ERROR) {
		return make_unexpected(std::format("setsockopt(IP_MULTICAST_LOOP) failed: {}", WSAGetLastError()));
	}

	UdpSocket udpSocket;
	udpSocket._platformSpecificData = new detail::PlatformSpecificSocketData{.socket = socketResult};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateReceiveOnlyMulticastSocket(const IpAddress& groupAddr, const Port& port) {
	EnsureWinsockInitialized();
	const auto socketResult = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketResult == INVALID_SOCKET) {
		return make_unexpected(std::format("socket failed: {}", WSAGetLastError()));
	}

	// There could be other clients in this machine, allow them to listen for incoming messages
	constexpr BOOL reuse = TRUE;
	if (::setsockopt(socketResult, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse)) == SOCKET_ERROR) {
		return make_unexpected(std::format("setsockopt(SO_REUSEADDR) failed: {}", WSAGetLastError()));
	}

	// Bind the socket to the local port
	sockaddr_in sin{};
	sin.sin_family = AF_INET;
	sin.sin_port = port.GetInNetworkOrder();
	sin.sin_addr.s_addr = INADDR_ANY;
	if (::bind(socketResult, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) == SOCKET_ERROR) {
		return make_unexpected(std::format("bind failed: {}", WSAGetLastError()));
	}

	// Join the multicast group on all interfaces
	ip_mreq mreq{};
	mreq.imr_multiaddr.s_addr = groupAddr.GetInNetworkOrder();
	mreq.imr_interface.s_addr = INADDR_ANY;
	if (::setsockopt(socketResult, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)) == SOCKET_ERROR) {
		return make_unexpected(std::format("setsockopt(IP_ADD_MEMBERSHIP) failed: {}", WSAGetLastError()));
	}

	UdpSocket udpSocket;
	udpSocket._selfPort = port;
	udpSocket._platformSpecificData = new detail::PlatformSpecificSocketData{.socket = socketResult};
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
		if (_platformSpecificData->socket != INVALID_SOCKET) {
			closesocket(_platformSpecificData->socket);
		}
		delete _platformSpecificData;
	}
}

m2::void_expected m2::network::UdpSocket::Send(const IpAddressAndPort& peerAddrAndPort, const uint8_t* buffer, const size_t length) {
	sockaddr_in sin{};
	sin.sin_family = AF_INET;
	sin.sin_port = peerAddrAndPort.port.GetInNetworkOrder();
	sin.sin_addr.s_addr = peerAddrAndPort.ipAddress.GetInNetworkOrder();

	const auto sendResult = ::sendto(_platformSpecificData->socket, reinterpret_cast<const char*>(buffer), I(length), 0, reinterpret_cast<sockaddr*>(&sin), sizeof(sin));
	if (sendResult == SOCKET_ERROR) {
		if (const auto lastError = WSAGetLastError(); lastError == WSAEWOULDBLOCK) {
			return make_unexpected("Blocked socket returned WSAEWOULDBLOCK");
		} else {
			return make_unexpected(std::format("sendto failed: {}", lastError));
		}
	}
	if (sendResult != I(length)) {
		return make_unexpected("Unable to send the whole UDP payload");
	}
	return {};
}

m2::expected<std::pair<int, m2::network::IpAddressAndPort>> m2::network::UdpSocket::Recv(uint8_t* buffer, const size_t length) {
	// Prepare source address
	sockaddr_in sin{};
	int slen = sizeof(sin);
	const auto recvResult = ::recvfrom(_platformSpecificData->socket, reinterpret_cast<char*>(buffer), I(length), 0, reinterpret_cast<sockaddr*>(&sin), &slen);
	if (recvResult == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			throw M2_ERROR("Blocking socket returned WSAEWOULDBLOCK for recv");
		}
		return make_unexpected(std::format("recvfrom failed: {}", WSAGetLastError()));
	}
	if (recvResult == 0) {
		return make_unexpected("Unexpected recvfrom result for a UDP socket: 0");
	}
	return std::make_pair(I(recvResult), IpAddressAndPort{
		.ipAddress = IpAddress::CreateFromNetworkOrder(sin.sin_addr.s_addr),
		.port = Port::CreateFromNetworkOrder(sin.sin_port)
	});
}
