#pragma once
#include "ServerActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>
#include <m2g_Lockstep.pb.h>

namespace m2::multiplayer::lockstep {
	class ServerActorInterface final : public ActorInterfaceBase<ServerActor, ServerActorInput, ServerActorOutput> {
		ServerActorOutput::ServerStateUpdate _serverStateUpdate{};

	public:
		explicit ServerActorInterface(const int maxClientCount) : ActorInterfaceBase(maxClientCount) {}

		// Accessors

		bool IsLobbyOpen();
		bool IsLobbyFrozen();

		// Modifiers

		void TryFreezeLobby(const m2g::pb::LockstepGameInitParams&);

	private:
		void ProcessOutbox();
	};
}
