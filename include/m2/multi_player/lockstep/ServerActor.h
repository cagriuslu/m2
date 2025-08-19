#pragma once
#include "ConnectionToClient.h"
#include "ServerActorInputOutput.h"
#include <m2/mt/actor/ActorBase.h>
#include <m2/multi_player/lockstep/MessagePasser.h>
#include <m2/ManagedObject.h>
#include <vector>
#include <memory>
#include <optional>

namespace m2::multiplayer::lockstep {
	class ServerActor final : ActorBase<ServerActorInput,ServerActorOutput> {
	public:
		struct Client {
			network::IpAddressAndPort address;
			std::unique_ptr<ConnectionToClient> client;
		};

		struct LobbyOpen {
			std::vector<Client> clients;
		};
		using State = std::variant<std::monostate, LobbyOpen>;

	private:
		const int _maxClientCount;
		std::optional<MessagePasser> _messagePasser;
		std::optional<ManagedObject<State>> _state;

	public:
		explicit ServerActor(const int maxClientCount) : ActorBase(), _maxClientCount(maxClientCount) {}
		~ServerActor() override = default;

		[[nodiscard]] const char* ThreadNameForLogging() const override { return "SR"; }

		bool Initialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override;

		bool operator()(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override;

		void Deinitialize(MessageBox<ServerActorInput>&, MessageBox<ServerActorOutput>&) override {}

	private:
		ConnectionToClient* FindClient(const network::IpAddressAndPort&);
	};
}
