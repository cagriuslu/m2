#pragma once
#include <m2g_Lockstep.pb.h>
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ServerActorInput {
		struct FreezeLobby {
			m2g::pb::LockstepGameInitParams gameInitParams;
		};
		std::variant<FreezeLobby> variant;
	};

	struct ServerActorOutput {
		struct ServerStateUpdate {
			size_t stateIndex{};
		};
		std::variant<ServerStateUpdate> variant;
	};
}
