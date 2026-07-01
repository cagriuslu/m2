#include "InMemoryNetworkRouter.h"

using namespace m2;
using namespace m2::network;
using namespace m2::network::detail;

InMemoryUdpRouter& InMemoryUdpRouter::Instance() {
	static InMemoryUdpRouter routerInstance;
	return routerInstance;
}

void InMemoryUdpRouter::RegisterInbox(const uint16_t networkOrderPort) {
	_inboxesByNetworkOrderPort[networkOrderPort] = {};
}
Port InMemoryUdpRouter::AllocateInbox() {
	while (_inboxesByNetworkOrderPort.contains(Port::CreateFromHostOrder(_nextEphemeralHostOrderPort).GetInNetworkOrder())) {
		++_nextEphemeralHostOrderPort;
	}
	const auto ephemeralPort = Port::CreateFromHostOrder(_nextEphemeralHostOrderPort++);
	_inboxesByNetworkOrderPort[ephemeralPort.GetInNetworkOrder()] = {};
	return ephemeralPort;
}
void InMemoryUdpRouter::UnregisterInbox(const uint16_t networkOrderPort) {
	_inboxesByNetworkOrderPort.erase(networkOrderPort);
}

void InMemoryUdpRouter::Deliver(const uint16_t destinationNetworkOrderPort, const InMemoryDatagram& datagram) {
	if (const auto inboxIt = _inboxesByNetworkOrderPort.find(destinationNetworkOrderPort); inboxIt != _inboxesByNetworkOrderPort.end()) {
		inboxIt->second.emplace_back(datagram);
	}
}
std::deque<InMemoryDatagram>* InMemoryUdpRouter::TryGetInbox(const uint16_t networkOrderPort) {
	const auto inboxIt = _inboxesByNetworkOrderPort.find(networkOrderPort);
	return inboxIt != _inboxesByNetworkOrderPort.end() ? &inboxIt->second : nullptr;
}

InMemoryTcpRouter& InMemoryTcpRouter::Instance() {
	static InMemoryTcpRouter routerInstance;
	return routerInstance;
}

void InMemoryTcpRouter::RegisterListener(const uint16_t networkOrderListenPort) {
	_connectionsPendingAcceptanceByNetworkOrderListenPort[networkOrderListenPort] = {};
}
void InMemoryTcpRouter::UnregisterListener(const uint16_t networkOrderListenPort) {
	_connectionsPendingAcceptanceByNetworkOrderListenPort.erase(networkOrderListenPort);
}
bool InMemoryTcpRouter::IsListening(const uint16_t networkOrderListenPort) const {
	return _connectionsPendingAcceptanceByNetworkOrderListenPort.contains(networkOrderListenPort);
}

std::shared_ptr<InMemoryTcpConnection> InMemoryTcpRouter::Connect(const uint16_t networkOrderListenPort, IpAddressAndPort& clientAddressAndPort) {
	const auto listenerIt = _connectionsPendingAcceptanceByNetworkOrderListenPort.find(networkOrderListenPort);
	if (listenerIt == _connectionsPendingAcceptanceByNetworkOrderListenPort.end()) {
		return nullptr; // No listener at this port: connection refused (the caller may retry later)
	}
	// Allocate an ephemeral client port so the server's accepted socket can address (and distinguish) this client. Two
	// clients connecting to the same server therefore get distinct addresses (same localhost IP, distinct ports), which
	// the server relies on to match reconnections.
	const auto clientPort = Port::CreateFromHostOrder(_nextEphemeralHostOrderPort++);
	clientAddressAndPort = IpAddressAndPort{IpAddress::CreateLocalhost(), clientPort};
	auto connection = std::make_shared<InMemoryTcpConnection>();
	listenerIt->second.emplace_back(InMemoryTcpPendingConnection{.clientAddressAndPort = clientAddressAndPort, .connection = connection});
	return connection;
}
std::optional<InMemoryTcpPendingConnection> InMemoryTcpRouter::Accept(const uint16_t networkOrderListenPort) {
	const auto listenerIt = _connectionsPendingAcceptanceByNetworkOrderListenPort.find(networkOrderListenPort);
	if (listenerIt == _connectionsPendingAcceptanceByNetworkOrderListenPort.end() || listenerIt->second.empty()) {
		return std::nullopt; // No connection is pending acceptance right now
	}
	auto pendingConnection = std::move(listenerIt->second.front());
	listenerIt->second.pop_front();
	return pendingConnection;
}
bool InMemoryTcpRouter::HasPendingConnection(const uint16_t networkOrderListenPort) const {
	const auto listenerIt = _connectionsPendingAcceptanceByNetworkOrderListenPort.find(networkOrderListenPort);
	return listenerIt != _connectionsPendingAcceptanceByNetworkOrderListenPort.end() && not listenerIt->second.empty();
}
