#include <m2/network/UdpSocket.h>
#include <m2/Log.h>
#include "PlatformSpecificSocketData.h"
#include "InMemoryNetworkRouter.h"
#include <algorithm>

m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateServerSideSocket(const Port& port) {
	// Register an inbox at the requested port so peers can address datagrams to this server.
	detail::InMemoryUdpRouter::Instance().RegisterInbox(port.GetInNetworkOrder());

	UdpSocket udpSocket;
	udpSocket._selfPort = port;
	udpSocket._platformSpecificUdpData = new detail::PlatformSpecificUdpSocketData{
		.selfAddressAndPort = IpAddressAndPort{IpAddress::CreateLocalhost(), port},
		.ownsInbox = true};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateClientSideSocket() {
	// Allocate a synthetic ephemeral port (and its inbox) so peers can reply to this client.
	const auto ephemeralPort = detail::InMemoryUdpRouter::Instance().AllocateInbox();

	UdpSocket udpSocket;
	udpSocket._platformSpecificUdpData = new detail::PlatformSpecificUdpSocketData{
		.selfAddressAndPort = IpAddressAndPort{IpAddress::CreateLocalhost(), ephemeralPort},
		.ownsInbox = true};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateSendOnlyMulticastSocket(const IpAddress&) {
	// Multicast discovery is a no-op on the web build, ie. this socket owns no inbox.
	UdpSocket udpSocket;
	udpSocket._platformSpecificUdpData = new detail::PlatformSpecificUdpSocketData{.selfAddressAndPort = {}, .ownsInbox = false};
	return std::move(udpSocket);
}
m2::expected<m2::network::UdpSocket> m2::network::UdpSocket::CreateReceiveOnlyMulticastSocket(const IpAddress&, const Port& port) {
	// Multicast discovery is a no-op on the web build, ie. this socket owns no inbox.
	UdpSocket udpSocket;
	udpSocket._selfPort = port;
	udpSocket._platformSpecificUdpData = new detail::PlatformSpecificUdpSocketData{
		.selfAddressAndPort = IpAddressAndPort{IpAddress::CreateLocalhost(), port},
		.ownsInbox = false};
	return std::move(udpSocket);
}
m2::network::UdpSocket::UdpSocket(UdpSocket&& other) noexcept {
	*this = std::move(other);
}
m2::network::UdpSocket& m2::network::UdpSocket::operator=(UdpSocket&& other) noexcept {
	std::swap(_platformSpecificUdpData, other._platformSpecificUdpData);
	std::swap(_selfPort, other._selfPort);
	return *this;
}
m2::network::UdpSocket::~UdpSocket() {
	if (_platformSpecificUdpData) {
		if (_platformSpecificUdpData->ownsInbox) {
			detail::InMemoryUdpRouter::Instance().UnregisterInbox(_platformSpecificUdpData->selfAddressAndPort.port.GetInNetworkOrder());
		}
		delete _platformSpecificUdpData;
	}
}

m2::void_expected m2::network::UdpSocket::Send(const IpAddressAndPort& peerAddrAndPort, const uint8_t* buffer, const size_t length) {
	detail::InMemoryUdpRouter::Instance().Deliver(peerAddrAndPort.port.GetInNetworkOrder(), detail::InMemoryDatagram{
		.sourceAddressAndPort = _platformSpecificUdpData->selfAddressAndPort,
		.payload = std::vector<uint8_t>{buffer, buffer + length}});
	return {};
}

m2::expected<std::pair<int, m2::network::IpAddressAndPort>> m2::network::UdpSocket::Recv(uint8_t* buffer, const size_t length) {
	auto* inbox = detail::InMemoryUdpRouter::Instance().TryGetInbox(_platformSpecificUdpData->selfAddressAndPort.port.GetInNetworkOrder());
	if (not inbox || inbox->empty()) {
		// Recv must only be called after Select reports the socket readable. An empty inbox here mirrors a blocking
		// socket returning EAGAIN on the POSIX implementation, which throws rather than silently returning nothing.
		throw M2_ERROR("Recv called on a UDP socket with an empty inbox");
	}
	const auto datagram = std::move(inbox->front());
	inbox->pop_front();
	const auto bytesToCopy = std::min(length, datagram.payload.size());
	std::copy_n(datagram.payload.cbegin(), bytesToCopy, buffer);
	return std::make_pair(I(bytesToCopy), datagram.sourceAddressAndPort);
}
