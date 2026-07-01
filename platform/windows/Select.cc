#include <m2/network/Select.h>
#include "PlatformSpecificSocketData.h"
#include <WinSock2.h>
#include <algorithm>
#include <functional>
#include <format>

#pragma comment(lib, "Ws2_32.lib")

using namespace m2;
using namespace m2::network;

namespace {
	template <typename SocketT>
	expected<std::optional<SelectResult<SocketT>>> WaitUntilSocketsReadyTemplate(const SocketHandles<SocketT>& readSockets,
			const SocketHandles<SocketT>& writeSockets, const uint64_t timeoutMs, const std::function<SOCKET(const SocketT*)>& socketFdGetter) {
		// Prepare read set for select
		fd_set read_set;
		FD_ZERO(&read_set);
		for (const auto& handle : readSockets) {
			FD_SET(socketFdGetter(handle), &read_set);
		}
		// Prepare write set for select
		fd_set write_set;
		FD_ZERO(&write_set);
		for (const auto& handle : writeSockets) {
			FD_SET(socketFdGetter(handle), &write_set);
		}
		// Prepare timeval
		timeval tv{};
		tv.tv_sec = static_cast<long>(timeoutMs / 1000);
		tv.tv_usec = static_cast<long>((timeoutMs % 1000) * 1000);

		// The first argument is ignored on Windows
		const int select_result = ::select(0, &read_set, &write_set, nullptr, &tv);
		if (select_result == SOCKET_ERROR) {
			return make_unexpected(std::format("select failed: {}", WSAGetLastError()));
		} else if (select_result == 0) {
			return std::nullopt; // Timeout occurred
		} else {
			// Prepare selected handles
			SelectResult<SocketT> selected_handles;
			for (const auto& handle : readSockets) {
				if (FD_ISSET(socketFdGetter(handle), &read_set)) {
					selected_handles.readableSockets.emplace_back(handle);
				}
			}
			for (const auto& handle : writeSockets) {
				if (FD_ISSET(socketFdGetter(handle), &write_set)) {
					selected_handles.writableSockets.emplace_back(handle);
				}
			}
			return std::move(selected_handles);
		}
	}
}

expected<std::optional<SelectResult<TcpSocket>>> Select::WaitUntilSocketsReady(const TcpSocketHandles& readSockets,
		const TcpSocketHandles& writeSockets, const uint64_t timeoutMs) {
	return WaitUntilSocketsReadyTemplate<TcpSocket>(readSockets, writeSockets, timeoutMs,
		[](const TcpSocket* s) -> SOCKET { return s->_platformSpecificTcpData->socket; });
}

expected<std::optional<SelectResult<UdpSocket>>> Select::WaitUntilSocketsReady(const UdpSocketHandles& readSockets,
		const UdpSocketHandles& writeSockets, const uint64_t timeoutMs) {
	return WaitUntilSocketsReadyTemplate<UdpSocket>(readSockets, writeSockets, timeoutMs,
		[](const UdpSocket* s) -> SOCKET { return s->_platformSpecificUdpData->socket; });
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
