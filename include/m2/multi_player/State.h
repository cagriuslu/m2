#pragma once
#include "../Object.h"

namespace m2::mplayer {
	class State {
		std::vector<ObjectId> _player_object_ids;

	public:

		// Modifiers
		void set_player_object_id(int player_idx, ObjectId id);
	};
}
