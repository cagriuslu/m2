#pragma once

#include <m2/Object.h>
#include <set>
#include "cuzn/Detail.h"

// Accessors
m2::Object* FindRoadAtLocation(m2g::pb::SpriteType location);
int LinkCountOfConnectionLocation(Connection conn);
int LinkCountOfRoadCharacter(const m2::Character& chr);

// Modifiers
void RemoveAllRoads();

// Filters
constexpr auto IsRoadCharacter = [](const m2::Character& chr) { return chr.owner().object_type() == m2g::pb::ROAD; };

// Transformers
constexpr auto ToCitiesOfRoadCharacter = [](const m2::Character& chr) -> std::set<m2g::pb::ItemType> {
	if (not IsRoadCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to canal or railroad");
	}
	std::set<m2g::pb::ItemType> city_cards;
	for (auto it = chr.FindItems(m2g::pb::ITEM_CATEGORY_CITY_CARD); it != chr.EndItems(); ++it) {
		city_cards.insert(it->type());
	}
	return city_cards;
};

m2::void_expected InitRoad(m2::Object& obj, Connection connection);
