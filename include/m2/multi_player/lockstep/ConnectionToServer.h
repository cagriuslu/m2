#pragma once
#include "ClientActorInputOutput.h"
#include "ConnectionToPeer.h"
#include "MessagePasser.h"
#include <m2/network/IpAddressAndPort.h>
#include <m2/mt/actor/MessageBox.h>
#include <m2/ManagedObject.h>
#include <m2g_Lockstep.pb.h>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ConnectionToServer final {
	public:
		class PeerList {
			std::vector<std::optional<ConnectionToPeer>> _peers;
			bool _connectionStateRequiresReporting{true};
		public:
			// Accessors

			auto begin() { return _peers.begin(); }
			auto end() { return _peers.end(); }
			auto cbegin() const { return _peers.cbegin(); }
			auto cend() const { return _peers.cend(); }
			std::optional<int> GetSelfIndex() const;
			int GetSize() const { return I(_peers.size()); }
			[[nodiscard]] std::optional<std::vector<std::deque<m2g::pb::LockstepPlayerInput>>> GetPeerPlayerInputsForTimecode(network::Timecode) const;

			// Modifiers

			ConnectionToPeer* Find(const network::IpAddressAndPort&);
			void Update(const pb::LockstepPeerDetails&, MessagePasser& messagePasser);
			void ReportIfAllPeersConnected(MessagePasser& messagePasser, const network::IpAddressAndPort& serverAddressAndPort);
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

		[[nodiscard]] const network::IpAddressAndPort& GetAddressAndPort() const { return _serverAddressAndPort; }
		[[nodiscard]] bool IsLobbyFrozen() const { return std::holds_alternative<LobbyFrozen>(_state.Get()); }
		[[nodiscard]] bool IsGameStarted() const { return std::holds_alternative<GameStarted>(_state.Get()); }
		[[nodiscard]] std::optional<int> GetSelfIndex() const;
		[[nodiscard]] int GetTotalPlayerCount() const;
		[[nodiscard]] std::optional<std::vector<std::deque<m2g::pb::LockstepPlayerInput>>> GetPeerPlayerInputsForTimecode(network::Timecode) const;

		// Modifiers

		void SetReadyState(bool readyState);
		void MarkGameAsStarted();
		void QueueOutgoingMessages(std::optional<network::Timecode> timecode, const std::deque<m2g::pb::LockstepPlayerInput>*);
		void DeliverIncomingMessage(pb::LockstepMessage&& msg);
		void DeliverIncomingMessageToPeer(MessageAndSender&& msg);

	private:
		void QueueOutgoingMessage(pb::LockstepMessage&& msg);
		void QueueOutgoingMessage(const pb::LockstepMessage& msg);
		void QueueOutgoingMessageToPeers(const pb::LockstepMessage& msg);
	};
}
