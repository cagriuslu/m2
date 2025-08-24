#pragma once
#include "ServerActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>
#include <m2g_Lockstep.pb.h>

namespace m2::multiplayer::lockstep {
	class ServerActorInterface final : public ActorInterfaceBase<ServerActor, ServerActorInput, ServerActorOutput> {
		const std::function<void(const ServerActorOutput&)> _stateUpdateProcessor;
		ServerActorOutput::ServerStateUpdate _serverStateUpdate{};

	public:
		explicit ServerActorInterface(int maxClientCount);

		// Accessors

		bool IsLobbyOpen();
		/// Checks if lobby freeze message is delivered to all clients. Blocks until the server actor responds.
		bool IsLobbyFrozen();

		// Modifiers

		void TryFreezeLobby(const m2g::pb::LockstepGameInitParams&);
	};
}
