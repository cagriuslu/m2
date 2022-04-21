#ifndef IMPL_EXPLOSIVEWEAPON_H
#define IMPL_EXPLOSIVEWEAPON_H

#include "m2/SpriteBlueprint.h"

namespace impl::character {
	enum ExplosiveStatus {
		EXPLOSIVE_STATUS_IN_FLIGHT,
		EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP,
		EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP
	};

	struct ExplosiveBlueprint;
	struct ExplosiveState {
		const ExplosiveBlueprint* blueprint;
		float projectile_ttl_s;
		ExplosiveStatus status;
		explicit ExplosiveState(const ExplosiveBlueprint* blueprint);
	};
	struct ExplosiveBlueprint {
		m2::SpriteIndex sprite_index;
		float projectile_speed_mps;
		float projectile_ttl_s;
		float projectile_body_radius_m;
		float damage_max;
		float damage_min;
		float damage_radius_m;
	};

	struct ExplosiveWeaponBlueprint;
	struct ExplosiveWeaponState {
		const ExplosiveWeaponBlueprint* blueprint;
		float cooldown_counter_s;
		unsigned explosive_count;
		explicit ExplosiveWeaponState(const ExplosiveWeaponBlueprint* blueprint);
		void process_time(float time_passed_s);
	};
	struct ExplosiveWeaponBlueprint {
		ExplosiveBlueprint explosive;
		float cooldown_s;
		unsigned initial_explosive_count;
	};

	extern const ExplosiveWeaponBlueprint explosive_weapon_grenade;
	extern const ExplosiveWeaponBlueprint explosive_weapon_grenade_launcher;
}

#endif //IMPL_EXPLOSIVEWEAPON_H
