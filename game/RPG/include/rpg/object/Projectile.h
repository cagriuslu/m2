#pragma once
#include <Item.pb.h>
#include <m2/Meta.h>

namespace rpg {
	m2::void_expected create_projectile(m2::Object& obj, const m2::VecF& intended_direction, const m2::Item& ranged_weapon, bool is_friend);
}
