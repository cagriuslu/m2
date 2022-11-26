#ifndef IMPL_PROJECTILE_H
#define IMPL_PROJECTILE_H

#include "rpg/RangedWeapon.h"
#include <m2/Value.h>

namespace obj {
	struct Projectile {
		static m2::VoidValue init(m2::Object& obj, const chr::ProjectileBlueprint* blueprint, m2::Vec2f dir);
	};
}

#endif //IMPL_PROJECTILE_H
