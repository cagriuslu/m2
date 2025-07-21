#pragma once
#include "Type.h"
#include <m2/network/ClientManager.h>
#include <m2/network/PingBroadcastThread.h>
#include <m2/network/Select.h>
#include <m2/network/TcpSocket.h>
#include <m2/mt/actor/ActorBase.h>
#include <variant>

namespace m2 {
	struct ServerActorInput {
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

	struct ServerActorOutput {
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

	class ServerActor final : ActorBase<ServerActorInput, ServerActorOutput> {
		using ReadAndWriteTcpSocketHandles = std::pair<network::TcpSocketHandles, network::TcpSocketHandles>;

		const mplayer::Type _type;
		const int _maxConnCount;

		expected<network::TcpSocket> _connectionListeningSocket{unexpect_t{}, "Uninitialized"};
		std::optional<network::PingBroadcastThread> _pingBroadcastThread;
		pb::ServerThreadState _state{pb::SERVER_INITIAL_STATE};
		std::vector<network::ClientManager> _clients;
		int _turnHolderIndex{};
		std::optional<pb::NetworkMessage> _lastServerUpdate;

	public:
		ServerActor(const mplayer::Type type, const int maxConnectionCount) : _type(type), _maxConnCount(maxConnectionCount) {}
		~ServerActor() override = default;

		const char* ThreadNameForLogging() const override { return "SR"; }

		bool Initialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override;

		bool operator()(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override;

		void Deinitialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override {}

	private:
		// Initialisation steps

		void CreateSocket();
		void BindSocket();
		void StartListening(MessageBox<ServerActorOutput>&);
		void StartPingBroadcast();

		// Polling steps

		void ProcessInbox(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&);
		void ProcessReceivedMessages(MessageBox<ServerActorOutput>&);
		void CheckDisconnectedClients(MessageBox<ServerActorOutput>&);
		static std::optional<ReadAndWriteTcpSocketHandles> SelectSockets(const ReadAndWriteTcpSocketHandles&);
		void CheckConnectionListeningSocket(MessageBox<ServerActorOutput>&, const network::TcpSocketHandles&);
		void CheckReadableClientSockets(const network::TcpSocketHandles&, MessageBox<ServerActorOutput>&);
		void CheckWritableClientSockets(const network::TcpSocketHandles&);

		ReadAndWriteTcpSocketHandles GetSocketHandlesToReadAndWrite();
		void SetStateAndPublish(MessageBox<ServerActorOutput>&, pb::ServerThreadState);
		void PublishStateUpdate(MessageBox<ServerActorOutput>&) const;
		void HandleDisconnectedClient(MessageBox<ServerActorOutput>&, int clientIndex);
		void HandleMisbehavedClient(MessageBox<ServerActorOutput>&, int clientIndex);
	};
}
