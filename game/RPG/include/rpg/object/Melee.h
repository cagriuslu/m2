#ifndef IMPL_MELEE_H
#define IMPL_MELEE_H

#include <Item.pb.h>
#include <m2/Value.h>

// TODO add bat, spear, dagger

namespace rpg {
	m2::VoidValue create_melee_object(m2::Object& obj, const m2::Vec2f& direction, const m2::Item& melee_weapon, bool is_friend);
}

#endif //IMPL_MELEE_H
