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
		struct WaitForPlayers {
			bool readyState{};
		};
		struct LobbyClose {};
		using State = std::variant<SearchForServer,WaitForPlayers,LobbyClose>;

	private:
		const network::IpAddressAndPort _serverAddressAndPort;
		MessagePasser& _messagePasser;
		ManagedObject<State> _state;

	public:
		ConnectionToServer(network::IpAddressAndPort serverAddress, MessagePasser& messagePasser, MessageBox<ClientActorOutput>& clientOutbox);

		// Accessors

		const network::IpAddressAndPort& GetAddressAndPort() const { return _serverAddressAndPort; }

		// Modifiers

		void SetReadyState(bool state);
		void QueueOutgoingMessages();
		void DeliverIncomingMessage(pb::LockstepMessage&& msg);

	private:
		void QueueOutgoingMessage(pb::LockstepMessage&& msg);
	};
}
