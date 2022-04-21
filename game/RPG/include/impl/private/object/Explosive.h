#ifndef IMPL_EXPLOSIVE_H
#define IMPL_EXPLOSIVE_H

#include <impl/private/character/ExplosiveWeapon.h>
#include <m2/Object.h>

namespace impl::object {
	struct Explosive {
		static M2Err init(m2::Object& obj, const character::ExplosiveBlueprint* blueprint, m2::ObjectID originator_id, m2::Vec2f position, m2::Vec2f direction);
	};
}

#endif //IMPL_EXPLOSIVE_H
