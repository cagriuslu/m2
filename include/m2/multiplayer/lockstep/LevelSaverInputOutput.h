#pragma once
#include <m2/network/Types.h>
#include <m2g_Lockstep.pb.h>
#include <deque>

namespace m2::multiplayer::lockstep {
	struct LevelSaverInput {
		struct PlayerInputs {
			network::Timecode timecode;
			std::vector<std::deque<m2g::pb::LockstepPlayerInput>> inputs;
		};
		struct DebugStateReport {
			network::Timecode timecode;
			pb::LockstepDebugStateReport report;
		};
		std::variant<PlayerInputs, DebugStateReport> variant;
	};
	struct LevelSaverOutput {};
}
