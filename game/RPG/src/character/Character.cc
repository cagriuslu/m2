#include "rpg/Character.h"
#include <m2g/Object.h>

chr::CharacterState::CharacterState(const CharacterBlueprint* blueprint) :
	blueprint(blueprint), stun_ttl_s() {
	if (blueprint->default_explosive_weapon) {
		explosive_weapon_state = ExplosiveWeaponState(blueprint->default_explosive_weapon);
	}
	if (blueprint->default_melee_weapon) {
		melee_weapon_state = MeleeWeaponState(blueprint->default_melee_weapon);
	}
	if (blueprint->default_ranged_weapon) {
		ranged_weapon_state = RangedWeaponState(blueprint->default_ranged_weapon);
	}
}

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
		.default_explosive_weapon = &explosive_weapon_grenade,
		.default_melee_weapon = &melee_weapon_bat,
		.default_ranged_weapon = &ranged_weapon_gun,
		.animation_type = m2g::pb::ANIMATION_TYPE_PLAYER_MOVEMENT
};

const chr::CharacterBlueprint chr::character_skeleton_000_chase = {
		.main_sprite = m2g::pb::SpriteType::ENEMY_LOOKDOWN_00,
		.default_explosive_weapon = nullptr,
		.default_melee_weapon = &melee_weapon_sword,
		.default_ranged_weapon = nullptr,
		.stun_ttl_s = 2.0f,
		.animation_type = m2g::pb::ANIMATION_TYPE_SKELETON_MOVEMENT,
		.aiBlueprint = &ai::chase_000,
};
