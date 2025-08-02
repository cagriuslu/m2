#pragma once
#include "MessagePasser.h"
#include "ControlMessage.h"
#include <m2/network/IpAddressAndPort.h>
#include <m2/Chrono.h>
#include <queue>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ConnectionToServer final {
		struct SearchingForServer {
			std::optional<Stopwatch> lastConnectionAttemptAt;
			unsigned responsesReceived{};
		};
		struct CheckingAdmittanceCriteria {};

		std::variant<SearchingForServer,CheckingAdmittanceCriteria> _state{};

	public:
		ConnectionToServer() = default;

		std::optional<ControlMessage> GatherOutgoingMessages(std::queue<pb::LockstepMessage>& out);

		std::optional<ControlMessage> DeliverIncomingMessage(pb::LockstepMessage&& in);
	};
}
