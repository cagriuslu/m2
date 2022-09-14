#include "rpg/MeleeWeapon.h"
#include <m2g/SpriteBlueprint.h>

chr::MeleeState::MeleeState(const MeleeBlueprint* blueprint) :
	blueprint(blueprint),
	ttl_s(blueprint->ttl_s) {}

chr::MeleeWeaponState::MeleeWeaponState(const MeleeWeaponBlueprint* blueprint) :
	blueprint(blueprint),
	cooldown_counter_s(0.0f) {}

void chr::MeleeWeaponState::process_time(float time_passed_s) {
	cooldown_counter_s += time_passed_s;
	if (blueprint->cooldown_s < cooldown_counter_s) {
		cooldown_counter_s = blueprint->cooldown_s + 0.001f;
	}
}

const chr::MeleeWeaponBlueprint chr::melee_weapon_bat = {
		.melee = {
				.sprite_index = m2g::IMPL_SPRITE_SWORD_00,
				.sprite = m2g::SpriteId::Sword,
				.damage = 60.0f,
				.motion = MELEE_MOTION_SWING,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.33f
};
const chr::MeleeWeaponBlueprint chr::melee_weapon_sword = {
		.melee = {
				.sprite_index = m2g::IMPL_SPRITE_SWORD_00,
				.sprite = m2g::SpriteId::Sword,
				.damage = 45.0f, // TODO normally 90
				.motion = MELEE_MOTION_SWING,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.33f
};
const chr::MeleeWeaponBlueprint chr::melee_weapon_spear = {
		.melee = {
				.sprite_index = m2g::IMPL_SPRITE_SWORD_00,
				.sprite = m2g::SpriteId::Sword,
				.damage = 120.0f,
				.motion = MELEE_MOTION_STAB,
				.ttl_s = 0.150f
		},
		.cooldown_s = 1.0f
};
const chr::MeleeWeaponBlueprint chr::melee_weapon_dagger = {
		.melee = {
				.sprite_index = m2g::IMPL_SPRITE_SWORD_00,
				.sprite = m2g::SpriteId::Sword,
				.damage = 60.0f,
				.motion = MELEE_MOTION_STAB,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.5f
};
