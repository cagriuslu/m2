#pragma once
#include "MessagePasser.h"
#include <m2/ProxyHelper.h>
#include <m2/network/Types.h>

namespace m2::multiplayer::lockstep {
	class ConnectionToPeer final {
		struct SearchForPeer {};
		struct ConnectedToPeer {
			std::map<network::Timecode, pb::LockstepPlayerInputs> _inputs;
		};
		using State = std::variant<SearchForPeer,ConnectedToPeer>;

		const network::IpAddressAndPort _addressAndPort;
		MessagePasser& _messagePasser;
		State _state;

	public:
		static constexpr auto InputCapacityInSeconds = 10;
		static inline auto InputCapacity = I(std::ceilf(m2g::LockstepGameTickFrequencyF)) * InputCapacityInSeconds;

		ConnectionToPeer(network::IpAddressAndPort address, MessagePasser& messagePasser) : _addressAndPort(std::move(address)), _messagePasser(messagePasser) {}

		// Accessors

		[[nodiscard]] const network::IpAddressAndPort& GetAddressAndPort() const { return _addressAndPort; }
		[[nodiscard]] bool IsConnected() const { return std::holds_alternative<ConnectedToPeer>(_state); }
		[[nodiscard]] bool DoPlayerInputsForTimecodeExist(network::Timecode) const;
		[[nodiscard]] std::optional<std::deque<m2g::pb::LockstepPlayerInput>> GetPlayerInputsForTimecode(network::Timecode) const;

		// Modifiers

		void QueueOutgoingMessages();
		void QueueOutgoingMessage(pb::LockstepMessage&& msg);
		void StorePlayerInputsReceivedFrom(const pb::LockstepPlayerInputs&);
	};
}
