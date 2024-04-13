#pragma once

#include <m2/Object.h>
#include <array>

namespace cuzn {
	// Filters
	constexpr auto is_road_character = [](m2::Character* chr) { return chr->parent().object_type() == m2g::pb::ROAD; };
	// Transformers
	constexpr auto to_city_cards_of_road_character = [](m2::Character* chr)  {
		if (not is_road_character(chr)) {
			throw M2ERROR("Character doesn't belong to canal or railroad");
		}
		auto it = chr->find_items(m2g::pb::ITEM_CATEGORY_CITY_CARD);
		auto first = it->type();
		auto second = (++it)->type();
		return std::make_pair(first, second);
	};

	// TODO init road

	m2::Object* find_road_at_location(m2g::pb::SpriteType location);
}
