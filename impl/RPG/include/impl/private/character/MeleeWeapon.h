#ifndef IMPL_MELEEWEAPON_H
#define IMPL_MELEEWEAPON_H

#include <m2/SpriteBlueprint.h>

namespace impl::character {
	enum MeleeMotion {
		MELEE_MOTION_SWING,
		MELEE_MOTION_STAB
	};

	struct MeleeBlueprint;
	struct MeleeState {
		const MeleeBlueprint* blueprint;
		float ttl_s;
		explicit MeleeState(const MeleeBlueprint* blueprint);
	};
	struct MeleeBlueprint {
		m2::SpriteIndex sprite_index;
		float damage;
		MeleeMotion motion;
		float ttl_s;
		[[nodiscard]] MeleeState get_state() const;
	};

	struct MeleeWeaponBlueprint;
	struct MeleeWeaponState {
		const MeleeWeaponBlueprint* blueprint;
		float cooldown_counter_s;
		explicit MeleeWeaponState(const MeleeWeaponBlueprint* blueprint);
		void process_time(float time_passed_s);
	};
	struct MeleeWeaponBlueprint {
		MeleeBlueprint melee;
		float cooldown_s;
		[[nodiscard]] MeleeWeaponState get_state() const;
	};

	extern const MeleeWeaponBlueprint melee_weapon_bat;
	extern const MeleeWeaponBlueprint melee_weapon_sword;
	extern const MeleeWeaponBlueprint melee_weapon_spear;
	extern const MeleeWeaponBlueprint melee_weapon_dagger;
}

#endif //IMPL_MELEEWEAPON_H