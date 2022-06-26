#ifndef IMPL_MELEE_H
#define IMPL_MELEE_H

#include "rpg/MeleeWeapon.h"
#include <m2/Value.h>

namespace obj {
	struct Melee {
		static m2::VoidValue init(m2::Object& obj, const chr::MeleeBlueprint* blueprint, m2::ID originatorId, m2::Vec2f position, m2::Vec2f direction);
	};
}

#endif //IMPL_MELEE_H
