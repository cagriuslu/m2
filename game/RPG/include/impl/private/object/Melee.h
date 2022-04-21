#ifndef IMPL_MELEE_H
#define IMPL_MELEE_H

#include <impl/private/character/MeleeWeapon.h>

namespace impl::object {
	struct Melee {
		static M2Err init(m2::Object& obj, const character::MeleeBlueprint* blueprint, ID originatorId, m2::Vec2f position, m2::Vec2f direction);
	};
}

#endif //IMPL_MELEE_H
