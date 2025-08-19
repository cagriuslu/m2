#pragma once
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ClientActorInput {};

	struct ClientActorOutput {
		struct ConnectionToServerStateUpdate {
			size_t stateIndex{};
		};
		std::variant<ConnectionToServerStateUpdate> variant;
	};
}
