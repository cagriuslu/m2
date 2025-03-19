#pragma once
#include <m2g_KeyType.pb.h>
#include "../math/VecF.h"

namespace m2 {
	enum CharacterMovementDirection {
		CHARMOVEMENT_NONE = 0,

		CHARMOVEMENT_DOWN_LEFT,
		CHARMOVEMENT_LEFT,
		CHARMOVEMENT_UP_LEFT,

		CHARMOVEMENT_DOWN_RIGHT,
		CHARMOVEMENT_RIGHT,
		CHARMOVEMENT_UP_RIGHT,

		CHARMOVEMENT_DOWN,
		CHARMOVEMENT_UP,
	};
	CharacterMovementDirection to_character_movement_direction(const VecF& direction_vector);

	std::pair<CharacterMovementDirection, VecF> calculate_character_movement(m2g::pb::KeyType left_key,
			m2g::pb::KeyType right_key, m2g::pb::KeyType up_key, m2g::pb::KeyType down_key);
}
