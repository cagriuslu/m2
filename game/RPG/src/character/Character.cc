#include "rpg/Character.h"
#include <m2g/Object.h>

chr::CharacterState::CharacterState(const CharacterBlueprint* blueprint) :
	blueprint(blueprint), stun_ttl_s() {}

void chr::CharacterState::process_time(float time_passed_s) {
	// Stun
	if (0.0f < stun_ttl_s) {
		stun_ttl_s -= time_passed_s;
		if (stun_ttl_s < 0.0f) {
			stun_ttl_s = 0.0f;
		}
	}
}

void chr::CharacterState::stun() {
	stun_ttl_s = blueprint->stun_ttl_s;
}
bool chr::CharacterState::is_stunned() const {
	return 0.0f < stun_ttl_s;
}

const chr::CharacterBlueprint chr::character_player = {
		.main_sprite = m2g::pb::SpriteType::PLAYER_LOOKDOWN_00,
		.animation_type = m2g::pb::ANIMATION_TYPE_PLAYER_MOVEMENT
};

const chr::CharacterBlueprint chr::character_skeleton_000_chase = {
		.main_sprite = m2g::pb::SpriteType::ENEMY_LOOKDOWN_00,
		.stun_ttl_s = 2.0f,
		.animation_type = m2g::pb::ANIMATION_TYPE_SKELETON_MOVEMENT,
		.aiBlueprint = &ai::chase_000,
};
