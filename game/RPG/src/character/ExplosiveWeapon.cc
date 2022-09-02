#include "rpg/ExplosiveWeapon.h"
#include <m2g/SpriteBlueprint.h>

chr::ExplosiveState::ExplosiveState(const ExplosiveBlueprint* blueprint) :
	blueprint(blueprint),
	projectile_ttl_s(blueprint->projectile_ttl_s),
	status(EXPLOSIVE_STATUS_IN_FLIGHT) {}

chr::ExplosiveWeaponState::ExplosiveWeaponState(const ExplosiveWeaponBlueprint* blueprint) :
	blueprint(blueprint),
	cooldown_counter_s(0.0f),
	explosive_count(blueprint->initial_explosive_count) {}

void chr::ExplosiveWeaponState::process_time(float time_passed_s) {
	cooldown_counter_s += time_passed_s;
	if (blueprint->cooldown_s < cooldown_counter_s) {
		cooldown_counter_s = blueprint->cooldown_s + 0.001f;
	}
}

const chr::ExplosiveWeaponBlueprint chr::explosive_weapon_grenade = {
		.explosive = {
				.sprite_index = m2g::IMPL_SPRITE_BOMB_00,
				.sprite = m2g::Sprite::Bomb,
				.projectile_speed_mps = 1.5f,
				.projectile_ttl_s = 4.0f,
				.projectile_body_radius_m = 0.25f,
				.damage_max = 100.0f,
				.damage_min = 15.0f,
				.damage_radius_m = 2.0f
		},
		.cooldown_s = 1.0f,
		.initial_explosive_count = 5
};
const chr::ExplosiveWeaponBlueprint chr::explosive_weapon_grenade_launcher = {
		.explosive = {
				.sprite_index = m2g::IMPL_SPRITE_BOMB_00,
				.sprite = m2g::Sprite::Bomb,
				.projectile_speed_mps = 2.5f,
				.projectile_ttl_s = 2.0f,
				.projectile_body_radius_m = 0.25f,
				.damage_max = 80.0f,
				.damage_min = 35.0f,
				.damage_radius_m = 0.75f
		},
		.cooldown_s = 1.0f,
		.initial_explosive_count = 5
};
