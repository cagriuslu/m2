#pragma once

#include <m2/Object.h>
#include <cuzn/Detail.h>

// Filters
constexpr auto is_factory_character = [](m2::Character& chr) { return chr.parent().object_type() == m2g::pb::FACTORY; };

// Transformers
constexpr auto to_city_card_of_factory_character = [](m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2ERROR("Character doesn't belong to a factory");
	}
	return chr.find_items(m2g::pb::ITEM_CATEGORY_CITY_CARD)->type();
};

// Accessors
m2::Object* find_factory_at_location(m2g::pb::SpriteType location);

m2::void_expected init_factory(m2::Object&, City, IndustryTile);
