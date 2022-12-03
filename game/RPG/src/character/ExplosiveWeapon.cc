#include "rpg/ExplosiveWeapon.h"
#include <m2g/Object.h>

chr::ExplosiveState::ExplosiveState(const ExplosiveBlueprint* blueprint) :
	blueprint(blueprint),
	projectile_ttl_s(blueprint->projectile_ttl_s),
	status(EXPLOSIVE_STATUS_IN_FLIGHT) {}

chr::ExplosiveWeaponState::ExplosiveWeaponState(const ExplosiveWeaponBlueprint* blueprint) :
	blueprint(blueprint) {}

const chr::ExplosiveWeaponBlueprint chr::explosive_weapon_grenade = {
		.explosive = {
				.sprite = m2g::pb::SpriteType::BOMB_00,
				.projectile_speed_mps = 1.5f,
				.projectile_ttl_s = 4.0f,
				.projectile_body_radius_m = 0.25f,
				.damage_max = 100.0f,
				.damage_min = 15.0f,
				.damage_radius_m = 2.0f
		},
		.initial_explosive_count = 5
};
const chr::ExplosiveWeaponBlueprint chr::explosive_weapon_grenade_launcher = {
		.explosive = {
				.sprite = m2g::pb::SpriteType::BOMB_00,
				.projectile_speed_mps = 2.5f,
				.projectile_ttl_s = 2.0f,
				.projectile_body_radius_m = 0.25f,
				.damage_max = 80.0f,
				.damage_min = 35.0f,
				.damage_radius_m = 0.75f
		},
		.initial_explosive_count = 5
};
