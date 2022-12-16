#ifndef RPG_RANGEDWEAPON_H
#define RPG_RANGEDWEAPON_H

#include <Item.pb.h>
#include <m2/Value.h>

namespace rpg {
	m2::VoidValue create_ranged_weapon_projectile(m2::Object& obj, const m2::Vec2f& intended_direction, const m2::Item& ranged_weapon);
}

#endif //RPG_RANGEDWEAPON_H
