#ifndef RPG_EXPLOSIVEWEAPON_H
#define RPG_EXPLOSIVEWEAPON_H

#include <m2/Object.h>
#include <m2/Value.h>
#include <Item.pb.h>

// TODO add grenade

namespace rpg {
	m2::VoidValue create_explosive_object(m2::Object& obj, const m2::Vec2f& intended_direction, const m2::Item& explosive_weapon);
}

#endif //RPG_EXPLOSIVEWEAPON_H
