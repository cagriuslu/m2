#pragma once
#include "TcpSocket.h"
#include "../Meta.h"
#include <optional>

namespace m2::network {
	using TcpSocketHandles = std::vector<TcpSocket*>;
	struct Select {
		/// Returns unexpected if an error occurs.
		/// Otherwise returns std::nullopt if timeout occurred.
		/// Otherwise returns pair<readable_sockets, writeable_sockets>.
		expected<std::optional<std::pair<TcpSocketHandles, TcpSocketHandles>>> operator()
		        (const TcpSocketHandles& sockets_to_read, const TcpSocketHandles& sockets_to_write, uint64_t timeout_ms);
	};
}
