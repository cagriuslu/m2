#pragma once
#include <m2/network/Types.h>
#include "InMemoryNetworkRouter.h"
#include <cstdint>
#include <memory>

namespace m2::network::detail {
	enum class TcpSocketRole {
		None,
		Listener,  /// A server-side listening socket waiting for incoming connections.
		ClientEnd, /// The connecting (client) end of an established connection.
		ServerEnd, /// The accepted (server) end of an established connection.
	};

	struct PlatformSpecificTcpSocketData {
		TcpSocketRole tcpRole{TcpSocketRole::None};
		/// A Listener socket owns the listener registered at `listeningNetworkOrderPort`
		uint16_t listeningNetworkOrderPort{};
		/// ClientEnd/ServerEnd sockets shares the bidirectional `tcpConnection` byte stream with its peer
		std::shared_ptr<InMemoryTcpConnection> tcpConnection;
	};

	struct PlatformSpecificUdpSocketData {
		IpAddressAndPort selfAddressAndPort{};
		/// Whether registered an inbox in the UDP router (and must therefore unregister it on close). Multicast sockets
		/// own no inbox, they are no-op.
		bool ownsInbox{};
	};
}
