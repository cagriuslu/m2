#pragma once
#include "ServerActorInterface.h"
#include "ClientActorInterface.h"
#include "LevelSaverInterface.h"
#include <m2/network/discovery/NetworkDiscoveryActorInterface.h>
#include <list>
#include <optional>

namespace m2::multiplayer::lockstep {
	struct ServerComponents {
		std::optional<network::discovery::NetworkDiscoveryActorInterface> networkDiscoveryActorInterface;
		std::optional<ServerActorInterface> serverActorInterface;
		std::optional<ClientActorInterface> hostClientActorInterface;
		std::list<ClientActorInterface> botClientActorInterfaces;
		std::optional<LevelSaverInterface> levelSaverInterface;
	};
}
