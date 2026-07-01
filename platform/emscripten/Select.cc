#include <m2/network/Select.h>
#include "PlatformSpecificSocketData.h"
#include "InMemoryNetworkRouter.h"

using namespace m2;
using namespace m2::network;

expected<std::optional<SelectResult<TcpSocket>>> Select::WaitUntilSocketsReady(const TcpSocketHandles& readSockets,
		const TcpSocketHandles& writeSockets, const uint64_t) {
	// Non-blocking on the web build: the timeout is ignored and current readiness is returned immediately so cooperative
	// pumping never blocks the single thread. A listening socket is readable when a connection is pending acceptance; a
	// connected socket is readable when its inbound stream has bytes or the peer end has closed (so recv() can report
	// EOF). Every socket is always writable (the in-process streams never back-pressure).
	SelectResult<TcpSocket> selectedHandles;
	for (auto* readSocket : readSockets) {
		const auto& socketData = *readSocket->_platformSpecificTcpData;
		bool isReadable = false;
		if (socketData.tcpRole == detail::TcpSocketRole::Listener) {
			isReadable = detail::InMemoryTcpRouter::Instance().HasPendingConnection(socketData.listeningNetworkOrderPort);
		} else if (socketData.tcpConnection) {
			const bool isClientEnd = socketData.tcpRole == detail::TcpSocketRole::ClientEnd;
			const auto& streamFromPeer = isClientEnd
				? socketData.tcpConnection->serverToClientBytes : socketData.tcpConnection->clientToServerBytes;
			const bool isPeerEndOpen = isClientEnd
				? socketData.tcpConnection->serverEndOpen : socketData.tcpConnection->clientEndOpen;
			isReadable = not streamFromPeer.empty() || not isPeerEndOpen;
		}
		if (isReadable) {
			selectedHandles.readableSockets.emplace_back(readSocket);
		}
	}
	for (auto* writeSocket : writeSockets) {
		selectedHandles.writableSockets.emplace_back(writeSocket);
	}
	if (selectedHandles.readableSockets.empty() && selectedHandles.writableSockets.empty()) {
		return std::nullopt; // Nothing ready (mirrors a POSIX select timeout with an empty result).
	}
	return std::move(selectedHandles);
}

expected<std::optional<SelectResult<UdpSocket>>> Select::WaitUntilSocketsReady(const UdpSocketHandles& readSockets,
		const UdpSocketHandles& writeSockets, const uint64_t) {
	// Non-blocking on the web build: ignore the timeout and report current readiness immediately. A UDP socket is
	// readable when its router inbox is non-empty; every socket is always writable.
	SelectResult<UdpSocket> selectedHandles;
	for (auto* readSocket : readSockets) {
		const auto* inbox = detail::InMemoryUdpRouter::Instance().TryGetInbox(readSocket->_platformSpecificUdpData->selfAddressAndPort.port.GetInNetworkOrder());
		if (inbox && not inbox->empty()) {
			selectedHandles.readableSockets.emplace_back(readSocket);
		}
	}
	for (auto* writeSocket : writeSockets) {
		selectedHandles.writableSockets.emplace_back(writeSocket);
	}
	if (selectedHandles.readableSockets.empty() && selectedHandles.writableSockets.empty()) {
		return std::nullopt; // Nothing ready (mirrors a POSIX select timeout with an empty result).
	}
	return std::move(selectedHandles);
}

expected<bool> Select::IsSocketReadable(UdpSocket* socket) {
	const auto selectResult = WaitUntilSocketsReady(UdpSocketHandles{socket}, UdpSocketHandles{}, 0);
	m2ReflectUnexpected(selectResult);
	return selectResult->has_value() && std::ranges::contains(selectResult->value().readableSockets, socket);
}
expected<bool> Select::IsSocketWritable(UdpSocket* socket) {
	const auto selectResult = WaitUntilSocketsReady(UdpSocketHandles{}, UdpSocketHandles{socket}, 0);
	m2ReflectUnexpected(selectResult);
	return selectResult->has_value() && std::ranges::contains(selectResult->value().writableSockets, socket);
}
