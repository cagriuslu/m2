#ifndef M2_CHARACTERMOVEMENT_H
#define M2_CHARACTERMOVEMENT_H

#include "../Controls.h"
#include "../Vec2f.h"

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

	std::pair<CharacterMovementDirection, Vec2f> calculate_character_movement(Key left_key, Key right_key, Key up_key, Key down_key);
}

#endif //M2_CHARACTERMOVEMENT_H
