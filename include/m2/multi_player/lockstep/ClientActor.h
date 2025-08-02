#pragma once
#include "ConnectionToServer.h"
#include "MessagePasser.h"
#include <m2/mt/actor/ActorBase.h>
#include <m2/network/Select.h>

namespace m2::multiplayer::lockstep {
	struct ClientActorInput {};
	struct ClientActorOutput {};

	class ClientActor final : ActorBase<ClientActorInput,ClientActorOutput> {
		const network::IpAddressAndPort _serverAddressAndPort;
		std::optional<MessagePasser> _messagePasser;

		// Connections

		std::optional<ConnectionToServer> _serverConnection;

	public:
		explicit ClientActor(network::IpAddressAndPort serverAddress) : ActorBase(), _serverAddressAndPort(std::move(serverAddress)) {}
		~ClientActor() override = default;

		[[nodiscard]] const char* ThreadNameForLogging() const override { return "CL"; }

		bool Initialize(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>&) override;

		bool operator()(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>&) override;

		void Deinitialize(MessageBox<ClientActorInput>&, MessageBox<ClientActorOutput>&) override {}

	private:
	};
}
