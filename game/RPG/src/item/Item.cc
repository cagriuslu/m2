#include "rpg/Item.h"
#include <m2g/SpriteBlueprint.h>

using namespace itm;

itm::ItemState::ItemState(const ItemBlueprint &blueprint) : blueprint(blueprint) {}

const itm::ItemBlueprint itm::health_drop_20 = {
		.drop_sprite_index = m2g::IMPL_SPRITE_RED_POTION_00,
		.buffs = {{chr::Attribute::HP, 20.0f}}
};
