#ifndef IMPL_PROJECTILE_H
#define IMPL_PROJECTILE_H

#include <impl/private/character/RangedWeapon.h>

namespace impl::object {
	struct Projectile {
		static M2Err init(m2::Object* obj, const character::ProjectileBlueprint* blueprint, ID originatorId, m2::Vec2f pos, m2::Vec2f dir);
	};
}

#endif //IMPL_PROJECTILE_H
