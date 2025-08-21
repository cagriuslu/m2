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
		class ClientList {
			std::vector<ConnectionToClient> _clients;
		public:
			ConnectionToClient* Find(const network::IpAddressAndPort&);
			ConnectionToClient* Add(const network::IpAddressAndPort&, MessagePasser&);
		};

	public:
		struct LobbyOpen {
			ClientList clientList;
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
	};
}
