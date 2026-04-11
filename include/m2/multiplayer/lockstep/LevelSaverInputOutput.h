#pragma once
#include <m2/network/Types.h>
#include <m2g_Lockstep.pb.h>
#include <deque>

namespace m2::multiplayer::lockstep {
	struct LevelSaverInput {
		network::Timecode timecode;
		std::vector<std::deque<m2g::pb::LockstepPlayerInput>> playerInputs;
	};
	struct LevelSaverOutput {};
}
