#pragma once

#include <m2/Object.h>
#include <set>

namespace cuzn {
	// Filters
	constexpr auto is_road_character = [](m2::Character* chr) { return chr->parent().object_type() == m2g::pb::ROAD; };
	// Transformers
	constexpr auto to_city_cards_of_road_character = [](m2::Character* chr) -> std::set<m2g::pb::ItemType> {
		if (not is_road_character(chr)) {
			throw M2ERROR("Character doesn't belong to canal or railroad");
		}
		std::set<m2g::pb::ItemType> city_cards;
		for (auto it = chr->find_items(m2g::pb::ITEM_CATEGORY_CITY_CARD); it != chr->end_items(); ++it) {
			city_cards.insert(it->type());
		}
		return city_cards;
	};

	// TODO init road

	m2::Object* find_road_at_location(m2g::pb::SpriteType location);
}
