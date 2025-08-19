#pragma once
#include "ClientActorInputOutput.h"
#include "ConnectionStatistics.h"
#include "MessagePasser.h"
#include <m2/network/IpAddressAndPort.h>
#include <m2/mt/actor/MessageBox.h>
#include <m2/Chrono.h>
#include <m2/ManagedObject.h>
#include <queue>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ConnectionToServer final {
	public:
		struct SearchForServer {};
		struct WaitForPlayers {};
		using State = std::variant<SearchForServer,WaitForPlayers>;

	private:
		ManagedObject<State> _state;

	public:
		explicit ConnectionToServer(MessageBox<ClientActorOutput>& clientOutbox);

		void GatherOutgoingMessages(const ConnectionStatistics*, std::queue<pb::LockstepMessage>& out);

		void DeliverIncomingMessage(const ConnectionStatistics*, pb::LockstepMessage&& in);
	};
}
