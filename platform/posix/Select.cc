#include <m2/network/Select.h>
#include "PlatformSpecificTcpSocketData.h"
#include <sys/select.h>
#include <sys/types.h>

m2::expected<std::optional<std::pair<m2::network::TcpSocketHandles, m2::network::TcpSocketHandles>>> m2::network::Select::operator()
		(const TcpSocketHandles& sockets_to_read, const TcpSocketHandles& sockets_to_write, uint64_t timeout_ms) {
	int max_fd = 0;

	// Prepare read set for select
	fd_set read_set;
	FD_ZERO(&read_set);
	for (const auto& handle : sockets_to_read) {
		auto fd = handle->_platform_specific_data->fd;
		FD_SET(fd, &read_set);
		max_fd = std::max(max_fd, fd);
	}
	// Prepare write set for select
	fd_set write_set;
	FD_ZERO(&write_set);
	for (const auto& handle : sockets_to_write) {
		auto fd = handle->_platform_specific_data->fd;
		FD_SET(fd, &write_set);
		max_fd = std::max(max_fd, fd);
	}
	// Prepare timeval
	timeval tv{};
	tv.tv_sec = static_cast<int64_t>(timeout_ms) / 1000;
	tv.tv_usec = static_cast<int32_t>((timeout_ms % 1000) * 1000);

	int select_result = ::select(max_fd + 1, &read_set, &write_set, nullptr, &tv);
	if (select_result == -1) {
		return make_unexpected(strerror(errno));
	} else if (select_result == 0) {
		return std::nullopt;
	} else {
		// Prepare selected handles
		std::pair<TcpSocketHandles, TcpSocketHandles> selected_handles;
		for (const auto& handle : sockets_to_read) {
			if (FD_ISSET(handle->_platform_specific_data->fd, &read_set)) {
				selected_handles.first.emplace_back(handle);
			}
		}
		for (const auto& handle : sockets_to_write) {
			if (FD_ISSET(handle->_platform_specific_data->fd, &write_set)) {
				selected_handles.second.emplace_back(handle);
			}
		}
		return std::move(selected_handles);
	}
}
