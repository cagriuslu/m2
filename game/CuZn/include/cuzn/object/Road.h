#pragma once

#include <m2/Object.h>
#include <set>
#include "cuzn/Detail.h"

// Accessors
m2::Object* find_road_at_location(m2g::pb::SpriteType location);

// Modifiers
void remove_all_roads();

// Filters
constexpr auto is_road_character = [](m2::Character& chr) { return chr.parent().object_type() == m2g::pb::ROAD; };

// Transformers
constexpr auto to_city_cards_of_road_character = [](m2::Character& chr) -> std::set<m2g::pb::ItemType> {
	if (not is_road_character(chr)) {
		throw M2_ERROR("Character doesn't belong to canal or railroad");
	}
	std::set<m2g::pb::ItemType> city_cards;
	for (auto it = chr.find_items(m2g::pb::ITEM_CATEGORY_CITY_CARD); it != chr.end_items(); ++it) {
		city_cards.insert(it->type());
	}
	return city_cards;
};
int link_count_of_road_character(m2::Character& chr);

m2::void_expected init_road(m2::Object& obj, Connection connection);
