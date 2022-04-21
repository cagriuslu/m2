#ifndef IMPL_RANGEDWEAPON_H
#define IMPL_RANGEDWEAPON_H

#include <m2/SpriteBlueprint.h>

namespace impl::character {
	struct ProjectileBlueprint;
	struct ProjectileState {
		const ProjectileBlueprint* blueprint;
		bool already_collided_this_step;
		float ttl_s;
		explicit ProjectileState(const ProjectileBlueprint* blueprint);
	};
	struct ProjectileBlueprint {
		m2::SpriteIndex sprite_index;
		float speed_mps;
		float damage;
		float ttl_s;
		float damage_accuracy;
		float ttl_accuracy;
	};

	struct RangedWeaponBlueprint;
	struct RangedWeaponState {
		const RangedWeaponBlueprint* blueprint;
		float cooldown_counter_s;
		explicit RangedWeaponState(const RangedWeaponBlueprint* blueprint);
		void process_time(float time_passed_s);
	};
	struct RangedWeaponBlueprint {
		ProjectileBlueprint projectile;
		unsigned projectile_count;
		float cooldown_s;
		float accuracy;
	};

	extern const RangedWeaponBlueprint ranged_weapon_gun; // Default
	extern const RangedWeaponBlueprint ranged_weapon_machinegun; // Fast, thus powerful
	extern const RangedWeaponBlueprint ranged_weapon_shotgun; // Slow but powerful
	extern const RangedWeaponBlueprint ranged_weapon_bow; // Slow, but piercing, thus default
}

#endif //IMPL_RANGEDWEAPON_H
