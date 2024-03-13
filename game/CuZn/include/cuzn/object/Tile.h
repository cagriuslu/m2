#pragma once

#include <m2/Object.h>

namespace cuzn {
	// Filters
	constexpr auto is_tile_character = [](m2::Character* chr) { return chr->parent().object_type() == m2g::pb::TILE; };
	// Transformers
	constexpr auto to_first_city_card_of_tile_character = [](m2::Character* chr) {
		if (not is_tile_character(chr)) {
			throw M2ERROR("Character doesn't belong to tile");
		}
		return chr->find_items(m2g::pb::ITEM_CATEGORY_CITY_CARD)->type();
	};

	// TODO init tile

	m2::Object* find_tile_at_location(m2g::pb::SpriteType location);
}
