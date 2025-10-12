#pragma once
#include "MessagePasser.h"
#include <m2/network/Types.h>

namespace m2::multiplayer::lockstep {
	class ConnectionToPeer final {
		const network::IpAddressAndPort _addressAndPort;
		MessagePasser& _messagePasser;

	public:
		ConnectionToPeer(network::IpAddressAndPort address, MessagePasser& messagePasser) : _addressAndPort(std::move(address)), _messagePasser(messagePasser) {}

		// Accessors

		[[nodiscard]] const network::IpAddressAndPort& GetAddressAndPort() const { return _addressAndPort; }
	};
}
