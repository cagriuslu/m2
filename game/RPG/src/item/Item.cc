#include <rpg/item/Item.h>
#include <m2g/SpriteBlueprint.h>

using namespace impl;
using namespace impl::item;

impl::item::ItemState::ItemState(const ItemBlueprint &blueprint) : blueprint(blueprint) {}

const impl::item::ItemBlueprint impl::item::health_drop_20 = {
		.drop_sprite_index = impl::IMPL_SPRITE_RED_POTION_00,
		.buffs = {{character::Attribute::HP, 20.0f}}
};
