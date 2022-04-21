#ifndef IMPL_MELEE_H
#define IMPL_MELEE_H

#include "rpg/MeleeWeapon.h"

namespace obj {
	struct Melee {
		static M2Err init(m2::Object& obj, const chr::MeleeBlueprint* blueprint, ID originatorId, m2::Vec2f position, m2::Vec2f direction);
	};
}

#endif //IMPL_MELEE_H
