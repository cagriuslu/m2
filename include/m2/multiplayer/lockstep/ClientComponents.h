#pragma once
#include "ClientActorInterface.h"
#include "LevelSaverInterface.h"
#include <optional>

namespace m2::multiplayer::lockstep {
	struct ClientComponents {
		ClientActorInterface guestClientActorInterface;
		std::optional<LevelSaverInterface> levelSaverInterface;

		explicit ClientComponents(network::IpAddressAndPort serverAddress) : guestClientActorInterface(std::move(serverAddress)) {}
	};
}
