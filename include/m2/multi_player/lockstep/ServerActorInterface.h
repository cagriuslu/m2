#pragma once
#include "ServerActor.h"
#include <m2/mt/actor/ActorInterfaceBase.h>

namespace m2::multiplayer::lockstep {
	class ServerActorInterface final : public ActorInterfaceBase<ServerActor, ServerActorInput, ServerActorOutput> {
	public:
		explicit ServerActorInterface(const int maxClientCount) : ActorInterfaceBase(maxClientCount) {}
	};
}
