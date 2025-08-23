#pragma once
#include <m2g_Lockstep.pb.h>
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ClientActorInput {
		struct SetReadyState {
			bool state;
		};
		std::variant<SetReadyState> variant;
	};

	struct ClientActorOutput {
		struct ConnectionToServerStateUpdate {
			size_t stateIndex{};
		};
		std::variant<ConnectionToServerStateUpdate> variant;
		std::optional<m2g::pb::LockstepGameInitParams> gameInitParams;
	};
}
