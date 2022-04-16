#include <impl/private/character/RangedWeapon.h>
#include <impl/public/SpriteBlueprint.h>
#include <m2/M2.h>

using namespace impl::character;

ProjectileState::ProjectileState(const ProjectileBlueprint *blueprint) :
	blueprint(blueprint),
	already_collided_this_step(false),
	ttl_s(m2::apply_accuracy(blueprint->ttl_s, blueprint->ttl_accuracy)) {}

ProjectileState ProjectileBlueprint::get_state() const {
	return ProjectileState{this};
}

RangedWeaponState::RangedWeaponState(const RangedWeaponBlueprint* blueprint) :
	blueprint(blueprint),
	cooldown_counter_s(0) {}

void RangedWeaponState::process_time(float time_passed_s) {
	cooldown_counter_s += time_passed_s;
	if (blueprint->cooldown_s < cooldown_counter_s) {
		cooldown_counter_s = blueprint->cooldown_s + 0.001f;
	}
}

RangedWeaponState RangedWeaponBlueprint::get_state() const {
	return RangedWeaponState{this};
}

const RangedWeaponBlueprint impl::character::ranged_weapon_gun = {
		.projectile = {
				.sprite_index = impl::IMPL_SPRITE_BULLET_00,
				.speed_mps = 20.0f,
				.damage = 35.0f,
				.ttl_s = 0.6f,
				.damage_accuracy = 0.8f,
				.ttl_accuracy = 0.9f
		},
		.projectile_count = 1,
		.cooldown_s = 0.1f,
		.accuracy = 0.95f
};
const RangedWeaponBlueprint impl::character::ranged_weapon_machinegun = {
		.projectile = {
				.sprite_index = impl::IMPL_SPRITE_BULLET_00,
				.speed_mps = 2.5f,
				.damage = 35.0f,
				.ttl_s = 3.0f,
				.damage_accuracy = 0.8f,
				.ttl_accuracy = 0.95f
		},
		.projectile_count = 1,
		.cooldown_s = 0.083f,
		.accuracy = 0.9f
};
const RangedWeaponBlueprint impl::character::ranged_weapon_shotgun = {
		.projectile = {
				.sprite_index = impl::IMPL_SPRITE_BULLET_01,
				.speed_mps = 2.0f,
				.damage = 20.0f,
				.ttl_s = 3.0f,
				.damage_accuracy = 0.8f,
				.ttl_accuracy = 0.95f
		},
		.projectile_count = 5,
		.cooldown_s = 1.0f,
		.accuracy = 0.9f
};
const RangedWeaponBlueprint impl::character::ranged_weapon_bow = {
		.projectile = {
				.sprite_index = impl::IMPL_SPRITE_BULLET_01,
				.speed_mps = 1.5f,
				.damage = 70.0f,
				.ttl_s = 5.0f,
				.damage_accuracy = 0.8f,
				.ttl_accuracy = 0.95f
		},
		.projectile_count = 1,
		.cooldown_s = 1.0f,
		.accuracy = 0.9f
};
