#include <m2/network/Select.h>
#include "PlatformSpecificSocketData.h"
#include <sys/select.h>
#include <sys/types.h>

using namespace m2;
using namespace m2::network;

namespace {
	template <typename SocketT>
	expected<std::optional<SelectResult<SocketT>>> WaitUntilSocketsReadyTemplate(const SocketHandles<SocketT>& readSockets,
			const SocketHandles<SocketT>& writeSockets, const uint64_t timeoutMs, const std::function<int(const SocketT*)>& socketFdGetter) {
		int max_fd = 0;

		// Prepare read set for select
		fd_set read_set;
		FD_ZERO(&read_set);
		for (const auto& handle : readSockets) {
			auto fd = socketFdGetter(handle);
			FD_SET(fd, &read_set);
			max_fd = std::max(max_fd, fd);
		}
		// Prepare write set for select
		fd_set write_set;
		FD_ZERO(&write_set);
		for (const auto& handle : writeSockets) {
			auto fd = socketFdGetter(handle);
			FD_SET(fd, &write_set);
			max_fd = std::max(max_fd, fd);
		}
		// Prepare timeval
		timeval tv{};
		tv.tv_sec = static_cast<int64_t>(timeoutMs) / 1000;
		tv.tv_usec = static_cast<int32_t>((timeoutMs % 1000) * 1000);

		const int select_result = ::select(max_fd + 1, &read_set, &write_set, nullptr, &tv);
		if (select_result == -1) {
			return make_unexpected(strerror(errno));
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
		[](const TcpSocket* s) -> int { return s->_platform_specific_data->fd; });
}

expected<std::optional<SelectResult<UdpSocket>>> Select::WaitUntilSocketsReady(const UdpSocketHandles& readSockets,
		const UdpSocketHandles& writeSockets, const uint64_t timeoutMs) {
	return WaitUntilSocketsReadyTemplate<UdpSocket>(readSockets, writeSockets, timeoutMs,
		[](const UdpSocket* s) -> int { return s->_platformSpecificData->fd; });
}
