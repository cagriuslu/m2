#pragma once
#include <Item.pb.h>
#include <m2/Value.h>

namespace rpg {
	/// Represents the blade object that's created by referring to the melee_weapon.
	m2::VoidValue create_blade(m2::Object& obj, const m2::VecF& direction, const m2::Item& melee_weapon, bool is_friend);
}
