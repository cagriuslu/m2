#pragma once
#include <variant>

namespace m2::multiplayer::lockstep {
	enum class ConnectionToServerState {
		SEARCHING_FOR_SERVER = 0, // Default state
		WAITING_FOR_PLAYERS,
	};

	struct ClientActorInput {};
	struct ClientActorOutput {
		struct ConnectionToServerStateUpdate {
			ConnectionToServerState state;
		};
		std::variant<ConnectionToServerStateUpdate> variant;
	};
}
