#pragma once
#include "ConnectionToClient.h"
#include <m2/mt/actor/ActorBase.h>
#include <m2/multi_player/lockstep/MessagePasser.h>
#include <m2/network/Select.h>
#include <vector>
#include <memory>
#include <optional>

namespace m2::multiplayer::lockstep {
	struct ServerActorInput {};
	struct ServerActorOutput {};

	class ServerActor final : ActorBase<ServerActorInput,ServerActorOutput> {
		struct ClientAndAddress {
			network::IpAddressAndPort address;
			std::unique_ptr<ConnectionToClient> client;
		};

		struct LobbyOpen {
			std::vector<ClientAndAddress> clients;
		};

		const int _maxClientCount;
		std::optional<MessagePasser> _messagePasser;
		std::queue<MessageAndSender> _receivedMessages;
		std::variant<std::monostate, LobbyOpen> _state;

	public:
		explicit ServerActor(const int maxClientCount) : ActorBase(), _maxClientCount(maxClientCount) {}
		~ServerActor() override = default;

		[[nodiscard]] const char* ThreadNameForLogging() const override { return "SR"; }

		bool Initialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override;

		bool operator()(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override;

		void Deinitialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override {}

	private:
		void PullIncomingMessages(const network::SocketHandles<network::UdpSocket>&);
		void ProcessOneReceivedMessage();

		ConnectionToClient* FindClient(const network::IpAddressAndPort&);
	};
}
