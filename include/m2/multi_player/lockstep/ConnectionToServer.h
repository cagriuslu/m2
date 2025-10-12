#pragma once
#include "ClientActorInputOutput.h"
#include "ConnectionStatistics.h"
#include "ConnectionToPeer.h"
#include "MessagePasser.h"
#include <m2/network/IpAddressAndPort.h>
#include <m2/mt/actor/MessageBox.h>
#include <m2/Chrono.h>
#include <m2/ManagedObject.h>
#include <m2g_Lockstep.pb.h>
#include <queue>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ConnectionToServer final {
	public:
		class PeerList {
			std::vector<std::optional<ConnectionToPeer>> _peers;
		public:
			// Modifiers

			void Update(const pb::LockstepPeerDetails&, MessagePasser& messagePasser);
		};

		struct SearchForServer {
			PeerList peerList;
		};
		struct WaitingInLobby {
			bool readyState{};
			PeerList peerList;
		};
		struct LobbyFrozen {
			m2g::pb::LockstepGameInitParams gameInitParams;
			PeerList peerList;
		};
		struct GameStarted {
			PeerList peerList;
		};
		using State = std::variant<SearchForServer,WaitingInLobby,LobbyFrozen,GameStarted>;

	private:
		const network::IpAddressAndPort _serverAddressAndPort;
		MessagePasser& _messagePasser;
		ManagedObject<State> _state;

	public:
		ConnectionToServer(network::IpAddressAndPort serverAddress, MessagePasser& messagePasser, MessageBox<ClientActorOutput>& clientOutbox);

		// Accessors

		const network::IpAddressAndPort& GetAddressAndPort() const { return _serverAddressAndPort; }

		// Modifiers

		void SetReadyState(bool readyState);
		void MarkGameAsStarted();
		void QueueOutgoingMessages(std::optional<network::Timecode> timecode, const std::deque<m2g::pb::LockstepPlayerInput>*);
		void DeliverIncomingMessage(pb::LockstepMessage&& msg);

	private:
		void QueueOutgoingMessage(pb::LockstepMessage&& msg);
	};
}
