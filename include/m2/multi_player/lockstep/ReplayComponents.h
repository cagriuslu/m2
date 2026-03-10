#pragma once
#include "LevelReplayer.h"

namespace m2::multiplayer::lockstep {
	struct ReplayComponents {
		int playerCount;
		int selfIndex;
		LevelReplayer levelReplayer;
	};
}
