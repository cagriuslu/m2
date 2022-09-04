#include "rpg/ConsumableBlueprint.h"
#include <m2g/SpriteBlueprint.h>

using namespace itm;

const itm::ConsumableBlueprint itm::health_drop_20 = {
		.drop_sprite = m2g::SpriteID::RedPotion,
		.buffs = {{chr::Attribute::HP, 20.0f}}
};
