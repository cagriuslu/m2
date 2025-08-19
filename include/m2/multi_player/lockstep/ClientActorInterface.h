#pragma once
#include "ClientActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>

namespace m2::multiplayer::lockstep {
	class ClientActorInterface final : public ActorInterfaceBase<ClientActor, ClientActorInput, ClientActorOutput> {
		ClientActorOutput::ConnectionToServerStateUpdate _connectionToServerState{};

	public:
		explicit ClientActorInterface(network::IpAddressAndPort serverAddress) : ActorInterfaceBase(std::move(serverAddress)) {}

		// Accessors

		bool IsSearchingForServer() { ProcessOutbox(); return _connectionToServerState.state == ConnectionToServerState::SEARCHING_FOR_SERVER; }
		bool IsWaitingForPlayers() { ProcessOutbox(); return _connectionToServerState.state == ConnectionToServerState::WAITING_FOR_PLAYERS; }

	private:
		void ProcessOutbox();
	};
}
