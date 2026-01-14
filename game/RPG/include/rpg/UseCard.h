#pragma once
#include <m2/component/Character.h>
#include <m2/Card.h>

namespace rpg {
	bool UseCard(m2::Character& chr, const m2::Card& card, float resource_multiplier = 1.0f);
}
