#pragma once
#include "MessagePasser.h"
#include "ConnectionStatistics.h"
#include <m2/network/IpAddressAndPort.h>
#include <m2/Chrono.h>
#include <queue>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ConnectionToServer final {
		struct SearchingForServer {};
		struct CheckingAdmittanceCriteria {};

		std::variant<SearchingForServer,CheckingAdmittanceCriteria> _state{};

	public:
		ConnectionToServer() = default;

		void GatherOutgoingMessages(ConnectionStatistics*, std::queue<pb::LockstepMessage>& out);

		void DeliverIncomingMessage(ConnectionStatistics*, pb::LockstepMessage&& in);
	};
}
