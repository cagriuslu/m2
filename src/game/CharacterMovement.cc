#include <m2/game/CharacterMovement.h>
#include <m2/Game.h>

m2::CharacterMovementDirection m2::to_character_movement_direction(const VecF& direction_vector) {
	int direction = CHARMOVEMENT_NONE;
	if (direction_vector.x < -0.5f) {
		direction = CHARMOVEMENT_LEFT;
		if (direction_vector.y < -0.5f) {
			direction += 1;
		} else if (0.5f < direction_vector.y) {
			direction -= 1;
		}
	} else if (0.5f < direction_vector.x) {
		direction = CHARMOVEMENT_RIGHT;
		if (direction_vector.y < -0.5f) {
			direction += 1;
		} else if (0.5f < direction_vector.y) {
			direction -= 1;
		}
	} else {
		if (direction_vector.y < -0.5f) {
			direction = CHARMOVEMENT_UP;
		} else if (0.5f < direction_vector.y) {
			direction = CHARMOVEMENT_DOWN;
		}
	}
	return static_cast<CharacterMovementDirection>(direction);
}

std::pair<m2::CharacterMovementDirection, m2::VecF> m2::calculate_character_movement(Key left_key, Key right_key, Key up_key, Key down_key) {
	VecF direction_vector;
	if ((bool)left_key && GAME.events.is_key_down(left_key)) {
		direction_vector.x -= 1.0f;
	}
	if ((bool)right_key && GAME.events.is_key_down(right_key)) {
		direction_vector.x += 1.0f;
	}
	if ((bool)up_key && GAME.events.is_key_down(up_key)) {
		direction_vector.y -= 1.0f;
	}
	if ((bool)down_key && GAME.events.is_key_down(down_key)) {
		direction_vector.y += 1.0f;
	}
	auto char_movement_dir = to_character_movement_direction(direction_vector);
	return {char_movement_dir, direction_vector.normalize()};
}
