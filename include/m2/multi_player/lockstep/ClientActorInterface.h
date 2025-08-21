#pragma once
#include "ClientActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>

namespace m2::multiplayer::lockstep {
	class ClientActorInterface final : public ActorInterfaceBase<ClientActor, ClientActorInput, ClientActorOutput> {
		ClientActorOutput::ConnectionToServerStateUpdate _connectionToServerState{};

	public:
		explicit ClientActorInterface(network::IpAddressAndPort serverAddress) : ActorInterfaceBase(std::move(serverAddress)) {}

		// Accessors

		bool IsSearchingForServer();
		bool IsWaitingForPlayers();

		// Modifiers

		void SetReadyState(bool state);

	private:
		void ProcessOutbox();
	};
}
