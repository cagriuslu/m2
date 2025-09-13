#include <m2/game/CharacterMovement.h>
#include <m2/Game.h>

m2::CharacterMovementDirection m2::to_character_movement_direction(const VecF& direction_vector) {
	int direction = CHARMOVEMENT_NONE;
	if (direction_vector.GetX() < -0.5f) {
		direction = CHARMOVEMENT_LEFT;
		if (direction_vector.GetY() < -0.5f) {
			direction += 1;
		} else if (0.5f < direction_vector.GetY()) {
			direction -= 1;
		}
	} else if (0.5f < direction_vector.GetX()) {
		direction = CHARMOVEMENT_RIGHT;
		if (direction_vector.GetY() < -0.5f) {
			direction += 1;
		} else if (0.5f < direction_vector.GetY()) {
			direction -= 1;
		}
	} else {
		if (direction_vector.GetY() < -0.5f) {
			direction = CHARMOVEMENT_UP;
		} else if (0.5f < direction_vector.GetY()) {
			direction = CHARMOVEMENT_DOWN;
		}
	}
	return static_cast<CharacterMovementDirection>(direction);
}

std::pair<m2::CharacterMovementDirection, m2::VecF> m2::calculate_character_movement(const m2g::pb::KeyType left_key,
		const m2g::pb::KeyType right_key, const m2g::pb::KeyType up_key, const m2g::pb::KeyType down_key) {
	VecF direction_vector;
	if (static_cast<bool>(left_key) && M2_GAME.events.IsKeyDown(left_key)) {
		direction_vector = {direction_vector.GetX() - 1.0f, direction_vector.GetY()};
	}
	if (static_cast<bool>(right_key) && M2_GAME.events.IsKeyDown(right_key)) {
		direction_vector = {direction_vector.GetX() + 1.0f, direction_vector.GetY()};
	}
	if (static_cast<bool>(up_key) && M2_GAME.events.IsKeyDown(up_key)) {
		direction_vector = direction_vector.WithY(direction_vector.GetY() - 1.0f);
	}
	if (static_cast<bool>(down_key) && M2_GAME.events.IsKeyDown(down_key)) {
		direction_vector = direction_vector.WithY(direction_vector.GetY() + 1.0f);
	}
	auto char_movement_dir = to_character_movement_direction(direction_vector);
	return {char_movement_dir, direction_vector.Normalize()};
}
