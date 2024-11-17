#include <m2/network/Select.h>
#include "PlatformSpecificTcpSocketData.h"
#include <winsock.h>

using namespace m2;
using namespace m2::network;

expected<std::optional<std::pair<TcpSocketHandles, TcpSocketHandles>>> Select::operator()
        (const TcpSocketHandles& sockets_to_read, const TcpSocketHandles& sockets_to_write, uint64_t timeout_ms) {
    // Prepare read set for select
    fd_set read_set;
    FD_ZERO(&read_set);
    for (const auto& handle : sockets_to_read) {
        auto fd = handle->_platform_specific_data->socket;
        FD_SET(fd, &read_set);
    }
    // Prepare write set for select
    fd_set write_set;
    FD_ZERO(&write_set);
    for (const auto& handle : sockets_to_write) {
        auto fd = handle->_platform_specific_data->socket;
        FD_SET(fd, &write_set);
    }
    // Prepare timeval
    timeval tv{};
    tv.tv_sec = static_cast<long>(timeout_ms) / 1000;
    tv.tv_usec = static_cast<long>((timeout_ms % 1000) * 1000);

    int select_result = ::select(0, &read_set, &write_set, nullptr, &tv);
    if (select_result == SOCKET_ERROR ) {
        return make_unexpected("select failed: " + std::to_string(WSAGetLastError()));
    } else if (select_result == 0) {
        return std::nullopt; // Timeout occurred
    } else {
        // Prepare selected handles
        std::pair<TcpSocketHandles, TcpSocketHandles> selected_handles;
        for (const auto& handle : sockets_to_read) {
            if (FD_ISSET(handle->_platform_specific_data->socket, &read_set)) {
                selected_handles.first.emplace_back(handle);
            }
        }
        for (const auto& handle : sockets_to_write) {
            if (FD_ISSET(handle->_platform_specific_data->socket, &write_set)) {
                selected_handles.second.emplace_back(handle);
            }
        }
        return std::move(selected_handles);
    }
}
