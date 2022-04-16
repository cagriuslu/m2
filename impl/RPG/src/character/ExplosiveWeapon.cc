#include <impl/private/character/ExplosiveWeapon.h>
#include <impl/public/SpriteBlueprint.h>

impl::character::ExplosiveState::ExplosiveState(const ExplosiveBlueprint* blueprint) :
	blueprint(blueprint),
	projectile_ttl_s(blueprint->projectile_ttl_s),
	status(EXPLOSIVE_STATUS_IN_FLIGHT) {}

impl::character::ExplosiveState impl::character::ExplosiveBlueprint::get_state() const {
	return ExplosiveState{this};
}

impl::character::ExplosiveWeaponState::ExplosiveWeaponState(const ExplosiveWeaponBlueprint* blueprint) :
	blueprint(blueprint),
	cooldown_counter_s(0.0f),
	explosive_count(blueprint->initial_explosive_count) {}

void impl::character::ExplosiveWeaponState::process_time(float time_passed_s) {
	cooldown_counter_s += time_passed_s;
	if (blueprint->cooldown_s < cooldown_counter_s) {
		cooldown_counter_s = blueprint->cooldown_s + 0.001f;
	}
}

impl::character::ExplosiveWeaponState impl::character::ExplosiveWeaponBlueprint::get_state() const {
	return ExplosiveWeaponState{this};
}

const impl::character::ExplosiveWeaponBlueprint impl::character::explosive_weapon_grenade = {
		.explosive = {
				.sprite_index = impl::IMPL_SPRITE_BOMB_00,
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
const impl::character::ExplosiveWeaponBlueprint impl::character::explosive_weapon_grenade_launcher = {
		.explosive = {
				.sprite_index = impl::IMPL_SPRITE_BOMB_00,
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
