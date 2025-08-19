#pragma once
#include "ClientActorInputOutput.h"
#include "ConnectionStatistics.h"
#include "MessagePasser.h"
#include <m2/network/IpAddressAndPort.h>
#include <m2/mt/actor/MessageBox.h>
#include <m2/Chrono.h>
#include <queue>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ConnectionToServer final {
		class StateManager {
			struct SearchForServer {};
			struct WaitForPlayers {};
			std::variant<SearchForServer,WaitForPlayers> _state{};
			MessageBox<ClientActorOutput>& _clientOutbox;

		public:
			explicit StateManager(MessageBox<ClientActorOutput>& clientOutbox) : _clientOutbox(clientOutbox) {}

			[[nodiscard]] bool ShouldSearchForServer() const { return std::holds_alternative<SearchForServer>(_state); }
			[[nodiscard]] bool ShouldWaitForPlayers() const { return std::holds_alternative<WaitForPlayers>(_state); }

			void MarkServerAsFound();
		} _stateManager;

	public:
		explicit ConnectionToServer(MessageBox<ClientActorOutput>& clientOutbox) : _stateManager(clientOutbox) {}

		void GatherOutgoingMessages(const ConnectionStatistics*, std::queue<pb::LockstepMessage>& out);

		void DeliverIncomingMessage(const ConnectionStatistics*, pb::LockstepMessage&& in);
	};
}
