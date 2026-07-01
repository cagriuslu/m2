#include <m2/network/TcpSocket.h>
#include <m2/network/Select.h>
#include "PlatformSpecificSocketData.h"
#include "InMemoryNetworkRouter.h"
#include <m2/Log.h>
#include <algorithm>

m2::expected<m2::network::TcpSocket> m2::network::TcpSocket::CreateServerSideSocket(const uint16_t port) {
	TcpSocket tcp_socket;
	tcp_socket._serverAddr = IpAddress{};
	tcp_socket._serverPort = Port::CreateFromHostOrder(port);
	tcp_socket._platformSpecificTcpData = new detail::PlatformSpecificTcpSocketData{
		.tcpRole = detail::TcpSocketRole::Listener,
		.listeningNetworkOrderPort = tcp_socket._serverPort.GetInNetworkOrder(),
		.tcpConnection = {}};
	return std::move(tcp_socket);
}
m2::expected<m2::network::TcpSocket> m2::network::TcpSocket::CreateClientSideSocket(const std::string& server_ip_addr, const uint16_t server_port) {
	TcpSocket tcp_socket;
	tcp_socket._serverAddr = IpAddress::CreateFromString(server_ip_addr);
	tcp_socket._serverPort = Port::CreateFromHostOrder(server_port);
	// A client socket owns no connection until connect() succeeds; until then its role is None.
	tcp_socket._platformSpecificTcpData = new detail::PlatformSpecificTcpSocketData{
		.tcpRole = detail::TcpSocketRole::None,
		.tcpConnection = {}};
	return std::move(tcp_socket);
}
m2::network::TcpSocket::TcpSocket(TcpSocket&& other) noexcept {
	*this = std::move(other);
}
m2::network::TcpSocket& m2::network::TcpSocket::operator=(TcpSocket&& other) noexcept {
	std::swap(_platformSpecificTcpData, other._platformSpecificTcpData);
	std::swap(_serverAddr, other._serverAddr);
	std::swap(_clientAddr, other._clientAddr);
	std::swap(_serverPort, other._serverPort);
	std::swap(_clientPort, other._clientPort);
	return *this;
}
m2::network::TcpSocket::~TcpSocket() {
	if (_platformSpecificTcpData) {
		// Release the router resources this socket owns so the next game starts clean: a listener unregisters; a
		// connected end marks itself closed so the peer's recv() can report EOF.
		if (_platformSpecificTcpData->tcpRole == detail::TcpSocketRole::Listener) {
			detail::InMemoryTcpRouter::Instance().UnregisterListener(_platformSpecificTcpData->listeningNetworkOrderPort);
		} else if (_platformSpecificTcpData->tcpConnection) {
			if (_platformSpecificTcpData->tcpRole == detail::TcpSocketRole::ClientEnd) {
				_platformSpecificTcpData->tcpConnection->clientEndOpen = false;
			} else if (_platformSpecificTcpData->tcpRole == detail::TcpSocketRole::ServerEnd) {
				_platformSpecificTcpData->tcpConnection->serverEndOpen = false;
			}
		}
		delete _platformSpecificTcpData;
	}
}

m2::expected<bool> m2::network::TcpSocket::bind() {
	if (not IsServerSideListeningSocket()) {
		throw M2_ERROR("Bind called on a non-listening non-server socket");
	}
	// There is no OS port to reserve in-process; the listener is registered in listen(). Binding always succeeds, so the
	// server's EADDRINUSE retry loop never spins on the web build.
	return true;
}
m2::void_expected m2::network::TcpSocket::listen(const int) {
	if (not IsServerSideListeningSocket()) {
		throw M2_ERROR("Listen called on a non-listening non-server socket");
	}
	detail::InMemoryTcpRouter::Instance().RegisterListener(_platformSpecificTcpData->listeningNetworkOrderPort);
	return {};
}
m2::expected<bool> m2::network::TcpSocket::connect() {
	if (not IsClientSideSocket()) {
		throw M2_ERROR("Connect called on a non-client socket");
	}
	// Succeeds instantly if a listener is registered at the destination port; otherwise reports a retryable refusal,
	// mirroring the POSIX semantics the client actor's state machine expects.
	IpAddressAndPort clientAddressAndPort;
	auto connection = detail::InMemoryTcpRouter::Instance().Connect(_serverPort.GetInNetworkOrder(), clientAddressAndPort);
	if (not connection) {
		return false; // No listener at the destination port: connection refused (the same socket may retry).
	}
	_platformSpecificTcpData->tcpRole = detail::TcpSocketRole::ClientEnd;
	_platformSpecificTcpData->tcpConnection = std::move(connection);
	return true;
}
m2::expected<std::optional<m2::network::TcpSocket>> m2::network::TcpSocket::accept() {
	if (not IsServerSideListeningSocket()) {
		throw M2_ERROR("Accept called on a non-listening non-server socket");
	}
	auto pendingConnection = detail::InMemoryTcpRouter::Instance().Accept(_platformSpecificTcpData->listeningNetworkOrderPort);
	if (not pendingConnection) {
		return std::nullopt; // No connection is pending; nothing to accept (mirrors a connection aborted before accept).
	}
	TcpSocket child_socket;
	child_socket._clientAddr = pendingConnection->clientAddressAndPort.ipAddress;
	child_socket._serverPort = _serverPort;
	child_socket._clientPort = pendingConnection->clientAddressAndPort.port;
	child_socket._platformSpecificTcpData = new detail::PlatformSpecificTcpSocketData{
		.tcpRole = detail::TcpSocketRole::ServerEnd,
		.tcpConnection = std::move(pendingConnection->connection)};
	return std::move(child_socket);
}

m2::expected<int> m2::network::TcpSocket::send(const uint8_t* buffer, const size_t length) {
	if (not IsServerSideConnectedSocket() && not IsClientSideSocket()) {
		throw M2_ERROR("Send called on a non-connected socket");
	}
	if (not _platformSpecificTcpData->tcpConnection) {
		throw M2_ERROR("Send called on a socket with no established connection");
	}
	// The client end writes into the client->server stream; the server end writes into the server->client stream.
	auto& connection = *_platformSpecificTcpData->tcpConnection;
	auto& streamToPeer = _platformSpecificTcpData->tcpRole == detail::TcpSocketRole::ClientEnd
		? connection.clientToServerBytes : connection.serverToClientBytes;
	streamToPeer.insert(streamToPeer.end(), buffer, buffer + length);
	return I(length);
}

m2::expected<int> m2::network::TcpSocket::recv(uint8_t* buffer, const size_t length) {
	if (not IsServerSideConnectedSocket() && not IsClientSideSocket()) {
		throw M2_ERROR("Recv called on a non-connected socket");
	}
	if (not _platformSpecificTcpData->tcpConnection) {
		throw M2_ERROR("Recv called on a socket with no established connection");
	}
	// The client end reads from the server->client stream; the server end reads from the client->server stream.
	auto& connection = *_platformSpecificTcpData->tcpConnection;
	const bool isClientEnd = _platformSpecificTcpData->tcpRole == detail::TcpSocketRole::ClientEnd;
	auto& streamFromPeer = isClientEnd ? connection.serverToClientBytes : connection.clientToServerBytes;
	const bool isPeerEndOpen = isClientEnd ? connection.serverEndOpen : connection.clientEndOpen;
	if (streamFromPeer.empty()) {
		if (not isPeerEndOpen) {
			return make_unexpected("Peer shut connection down"); // Mirrors POSIX recv() returning 0 (EOF).
		}
		return 0; // No data yet (mirrors a non-blocking recv returning EAGAIN/EWOULDBLOCK as 0).
	}
	const auto bytesToCopy = std::min(length, streamFromPeer.size());
	std::copy_n(streamFromPeer.cbegin(), bytesToCopy, buffer);
	streamFromPeer.erase(streamFromPeer.begin(), streamFromPeer.begin() + static_cast<std::ptrdiff_t>(bytesToCopy));
	return I(bytesToCopy);
}
