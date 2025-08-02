#pragma once
#include "ServerActorInterface.h"
#include "ClientActorInterface.h"
#include <optional>

namespace m2::multiplayer::lockstep {
	struct ServerComponents {
		std::optional<ServerActorInterface> serverActorInterface;
		std::optional<ClientActorInterface> hostClientActorInterface;
	};
}
