#pragma once
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ServerActorInput {
		struct CloseLobby {};
		std::variant<CloseLobby> variant;
	};

	struct ServerActorOutput {
		struct ServerStateUpdate {
			size_t stateIndex{};
		};
		std::variant<ServerStateUpdate> variant;
	};
}
