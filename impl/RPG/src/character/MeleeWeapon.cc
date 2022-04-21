#include <impl/private/character/MeleeWeapon.h>
#include <impl/public/SpriteBlueprint.h>

impl::character::MeleeState::MeleeState(const MeleeBlueprint* blueprint) :
	blueprint(blueprint),
	ttl_s(blueprint->ttl_s) {}

impl::character::MeleeWeaponState::MeleeWeaponState(const MeleeWeaponBlueprint* blueprint) :
	blueprint(blueprint),
	cooldown_counter_s(0.0f) {}

void impl::character::MeleeWeaponState::process_time(float time_passed_s) {
	cooldown_counter_s += time_passed_s;
	if (blueprint->cooldown_s < cooldown_counter_s) {
		cooldown_counter_s = blueprint->cooldown_s + 0.001f;
	}
}

const impl::character::MeleeWeaponBlueprint impl::character::melee_weapon_bat = {
		.melee = {
				.sprite_index = impl::IMPL_SPRITE_SWORD_00,
				.damage = 60.0f,
				.motion = MELEE_MOTION_SWING,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.33f
};
const impl::character::MeleeWeaponBlueprint impl::character::melee_weapon_sword = {
		.melee = {
				.sprite_index = impl::IMPL_SPRITE_SWORD_00,
				.damage = 45.0f, // TODO normally 90
				.motion = MELEE_MOTION_SWING,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.33f
};
const impl::character::MeleeWeaponBlueprint impl::character::melee_weapon_spear = {
		.melee = {
				.sprite_index = impl::IMPL_SPRITE_SWORD_00,
				.damage = 120.0f,
				.motion = MELEE_MOTION_STAB,
				.ttl_s = 0.150f
		},
		.cooldown_s = 1.0f
};
const impl::character::MeleeWeaponBlueprint impl::character::melee_weapon_dagger = {
		.melee = {
				.sprite_index = impl::IMPL_SPRITE_SWORD_00,
				.damage = 60.0f,
				.motion = MELEE_MOTION_STAB,
				.ttl_s = 0.150f
		},
		.cooldown_s = 0.5f
};
