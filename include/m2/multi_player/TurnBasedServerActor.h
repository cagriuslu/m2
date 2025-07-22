#pragma once
#include "Type.h"
#include <m2/network/TurnBasedClientManager.h>
#include <m2/network/PingBroadcastThread.h>
#include <m2/network/Select.h>
#include <m2/network/TcpSocket.h>
#include <m2/mt/actor/ActorBase.h>
#include <variant>

namespace m2 {
	struct TurnBasedServerActorInput {
		struct CloseLobby {};
		struct UpdateTurnHolder {
			int clientIndex;
		};
		struct SendServerUpdate {
			pb::NetworkMessage serverUpdate;
		};
		struct SendServerCommand {
			int receiverIndex; /// If negative, all clients should receive the command
			m2g::pb::ServerCommand serverCommand;
		};
		std::variant<CloseLobby,UpdateTurnHolder,SendServerUpdate,SendServerCommand> variant;
	};

	struct TurnBasedServerActorOutput {
		struct StateUpdate {
			pb::ServerThreadState threadState{pb::SERVER_INITIAL_STATE};
			int clientCount{}, readyClientCount{};
		};
		struct ClientEvent {
			struct CommandFromTurnHolder {
				pb::NetworkMessage turnHolderCommand;
			};
			struct DisconnectedClient {
				int clientIndex;
			};
			std::variant<CommandFromTurnHolder,DisconnectedClient> eventVariant;
		};
		std::variant<StateUpdate,ClientEvent> variant;
	};

	class TurnBasedServerActor final : ActorBase<TurnBasedServerActorInput, TurnBasedServerActorOutput> {
		using ReadAndWriteTcpSocketHandles = std::pair<network::TcpSocketHandles, network::TcpSocketHandles>;

		const mplayer::Type _type;
		const int _maxConnCount;

		expected<network::TcpSocket> _connectionListeningSocket{unexpect_t{}, "Uninitialized"};
		std::optional<network::PingBroadcastThread> _pingBroadcastThread;
		pb::ServerThreadState _state{pb::SERVER_INITIAL_STATE};
		std::vector<network::TurnBasedClientManager> _clients;
		int _turnHolderIndex{};
		std::optional<pb::NetworkMessage> _lastServerUpdate;

	public:
		TurnBasedServerActor(const mplayer::Type type, const int maxConnectionCount) : _type(type), _maxConnCount(maxConnectionCount) {}
		~TurnBasedServerActor() override = default;

		const char* ThreadNameForLogging() const override { return "SR"; }

		bool Initialize(MessageBox<TurnBasedServerActorInput>&, MessageBox<TurnBasedServerActorOutput>&) override;

		bool operator()(MessageBox<TurnBasedServerActorInput>&, MessageBox<TurnBasedServerActorOutput>&) override;

		void Deinitialize(MessageBox<TurnBasedServerActorInput>&, MessageBox<TurnBasedServerActorOutput>&) override {}

	private:
		// Initialisation steps

		void CreateSocket();
		void BindSocket();
		void StartListening(MessageBox<TurnBasedServerActorOutput>&);
		void StartPingBroadcast();

		// Polling steps

		void ProcessInbox(MessageBox<TurnBasedServerActorInput>&, MessageBox<TurnBasedServerActorOutput>&);
		void ProcessReceivedMessages(MessageBox<TurnBasedServerActorOutput>&);
		void CheckDisconnectedClients(MessageBox<TurnBasedServerActorOutput>&);
		static std::optional<ReadAndWriteTcpSocketHandles> SelectSockets(const ReadAndWriteTcpSocketHandles&);
		void CheckConnectionListeningSocket(MessageBox<TurnBasedServerActorOutput>&, const network::TcpSocketHandles&);
		void CheckReadableClientSockets(const network::TcpSocketHandles&, MessageBox<TurnBasedServerActorOutput>&);
		void CheckWritableClientSockets(const network::TcpSocketHandles&);

		ReadAndWriteTcpSocketHandles GetSocketHandlesToReadAndWrite();
		void SetStateAndPublish(MessageBox<TurnBasedServerActorOutput>&, pb::ServerThreadState);
		void PublishStateUpdate(MessageBox<TurnBasedServerActorOutput>&) const;
		void HandleDisconnectedClient(MessageBox<TurnBasedServerActorOutput>&, int clientIndex);
		void HandleMisbehavedClient(MessageBox<TurnBasedServerActorOutput>&, int clientIndex);
	};
}
