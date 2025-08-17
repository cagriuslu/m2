#pragma once
#include "MessagePasser.h"
#include "ConnectionStatistics.h"
#include <m2/network/IpAddressAndPort.h>
#include <m2/Chrono.h>
#include <queue>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ConnectionToServer final {
		struct SearchForServer {};
		struct WaitForPlayers {};

		std::variant<SearchForServer,WaitForPlayers> _state{};

	public:
		ConnectionToServer() = default;

		void GatherOutgoingMessages(const ConnectionStatistics*, std::queue<pb::LockstepMessage>& out);

		void DeliverIncomingMessage(const ConnectionStatistics*, pb::LockstepMessage&& in);
	};
}
