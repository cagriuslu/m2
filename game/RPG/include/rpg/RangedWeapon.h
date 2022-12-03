#ifndef IMPL_RANGEDWEAPON_H
#define IMPL_RANGEDWEAPON_H

#include "SpriteType.pb.h"

namespace chr {
	struct ProjectileBlueprint;
	struct ProjectileState {
		const ProjectileBlueprint* blueprint;
		bool already_collided_this_step;
		float ttl_s;
		explicit ProjectileState(const ProjectileBlueprint* blueprint);
	};
	struct ProjectileBlueprint {
		m2g::pb::SpriteType sprite;
		float speed_mps;
		float damage;
		float ttl_s;
		float damage_accuracy;
		float ttl_accuracy;
	};

	struct RangedWeaponBlueprint;
	struct RangedWeaponState {
		const RangedWeaponBlueprint* blueprint;
		explicit RangedWeaponState(const RangedWeaponBlueprint* blueprint);
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
