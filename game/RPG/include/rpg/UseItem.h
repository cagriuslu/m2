#pragma once
#include <m2/component/Character.h>
#include <m2/Item.h>

namespace rpg {
	bool UseItem(m2::Character& chr, const m2::Item& item, float resource_multiplier = 1.0f);
}
