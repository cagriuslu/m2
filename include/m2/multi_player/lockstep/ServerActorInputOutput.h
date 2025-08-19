#pragma once
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ServerActorInput {};

	struct ServerActorOutput {
		struct ServerStateUpdate {
			size_t stateIndex{};
		};
		std::variant<ServerStateUpdate> variant;
	};
}
