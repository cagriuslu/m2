#include "rpg/MeleeWeapon.h"
#include <m2g/Object.h>

chr::MeleeState::MeleeState(const MeleeBlueprint* blueprint) :
	blueprint(blueprint),
	ttl_s(blueprint->ttl_s) {}

chr::MeleeWeaponState::MeleeWeaponState(const MeleeWeaponBlueprint* blueprint) :
	blueprint(blueprint) {}

const chr::MeleeWeaponBlueprint chr::melee_weapon_bat = {
		.melee = {
				.sprite = m2g::pb::SpriteType::SWORD_00,
				.damage = 60.0f,
				.ttl_s = 0.150f
		}
};
const chr::MeleeWeaponBlueprint chr::melee_weapon_sword = {
		.melee = {
				.sprite = m2g::pb::SpriteType::SWORD_00,
				.damage = 45.0f, // TODO normally 90
				.ttl_s = 0.150f
		}
};
const chr::MeleeWeaponBlueprint chr::melee_weapon_spear = {
		.melee = {
				.sprite = m2g::pb::SpriteType::SWORD_00,
				.damage = 120.0f,
				.ttl_s = 0.150f
		}
};
const chr::MeleeWeaponBlueprint chr::melee_weapon_dagger = {
		.melee = {
				.sprite = m2g::pb::SpriteType::SWORD_00,
				.damage = 60.0f,
				.ttl_s = 0.150f
		}
};
