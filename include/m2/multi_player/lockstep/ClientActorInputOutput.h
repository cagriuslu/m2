#pragma once
#include <m2g_Lockstep.pb.h>
#include <deque>
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ClientActorInput {
		struct SetReadyState {
			bool state;
		};
		struct QueuePlayerInput {
			m2g::pb::LockstepPlayerInput playerInput;
		};
		std::variant<SetReadyState,QueuePlayerInput> variant;
	};

	struct ClientActorOutput {
		struct ConnectionToServerStateUpdate {
			size_t stateIndex{};
			std::optional<int> selfIndex;
			int totalPlayerCount{};
		};
		struct PlayerInputsToSimulate {
			std::vector<std::deque<m2g::pb::LockstepPlayerInput>> playerInputs;
		};
		std::variant<ConnectionToServerStateUpdate,PlayerInputsToSimulate> variant;
		std::optional<m2g::pb::LockstepGameInitParams> gameInitParams{};
	};
}
