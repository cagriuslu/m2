#pragma once
#include <Item.pb.h>
#include <m2/Value.h>

// TODO add bat, spear, dagger

namespace rpg {
	m2::VoidValue create_melee_object(m2::Object& obj, const m2::VecF& direction, const m2::Item& melee_weapon, bool is_friend);
}
