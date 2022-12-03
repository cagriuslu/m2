#ifndef IMPL_MELEEWEAPON_H
#define IMPL_MELEEWEAPON_H

#include "SpriteType.pb.h"

namespace chr {
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
		m2g::pb::SpriteType sprite;
		float damage;
		MeleeMotion motion;
		float ttl_s;
	};

	struct MeleeWeaponBlueprint;
	struct MeleeWeaponState {
		const MeleeWeaponBlueprint* blueprint;
		explicit MeleeWeaponState(const MeleeWeaponBlueprint* blueprint);
	};
	struct MeleeWeaponBlueprint {
		MeleeBlueprint melee;
	};

	extern const MeleeWeaponBlueprint melee_weapon_bat;
	extern const MeleeWeaponBlueprint melee_weapon_sword;
	extern const MeleeWeaponBlueprint melee_weapon_spear;
	extern const MeleeWeaponBlueprint melee_weapon_dagger;
}

#endif //IMPL_MELEEWEAPON_H
