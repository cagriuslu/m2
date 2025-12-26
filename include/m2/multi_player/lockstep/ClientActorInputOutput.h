#pragma once
#include <m2/network/Types.h>
#include <m2g_Lockstep.pb.h>
#include <deque>
#include <variant>

namespace m2::multiplayer::lockstep {
	struct ClientActorInput {
		struct SetReadyState {
			bool state;
		};
		struct QueueThisPlayerInput {
			std::deque<m2g::pb::LockstepPlayerInput> inputs;
		};
		struct GameStateHash {
			network::Timecode timecode;
			int32_t hash;
		};
		std::variant<SetReadyState,QueueThisPlayerInput, GameStateHash> variant;
	};

	struct ClientActorOutput {
		struct ConnectionToServerStateUpdate {
			size_t stateIndex{};
			std::optional<int> selfIndex;
			int totalPlayerCount{};
		};
		struct PlayerInputsToSimulate {
			network::Timecode timecode;
			std::vector<std::deque<m2g::pb::LockstepPlayerInput>> playerInputs;
		};
		std::variant<ConnectionToServerStateUpdate,PlayerInputsToSimulate> variant;
		std::optional<m2g::pb::LockstepGameInitParams> gameInitParams{};
	};
}
