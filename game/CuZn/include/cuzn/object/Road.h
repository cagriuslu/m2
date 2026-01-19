#pragma once

#include "cuzn/Detail.h"
#include <m2/component/character/FastCharacter.h>
#include <m2/Object.h>
#include <set>

// Accessors
m2::Object* FindRoadAtLocation(m2g::pb::SpriteType location);
int LinkCountOfConnectionLocation(Connection conn);
int LinkCountOfRoadCharacter(const m2::Character& chr);

// Modifiers
void RemoveAllRoads();

// Filters
constexpr auto IsRoadCharacter = [](const m2::Character& chr) { return chr.Owner().GetType() == m2g::pb::ROAD; };

// Transformers
constexpr auto ToCitiesOfRoadCharacter = [](const m2::Character& chr) -> std::set<m2g::pb::CardType> {
	if (not IsRoadCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to canal or railroad");
	}
	const auto cardTypes = dynamic_cast<const m2::FastCharacter&>(chr).GetCardTypes(m2g::pb::CARD_CATEGORY_CITY_CARD);
	return {cardTypes.begin(), cardTypes.end()};
};

m2::void_expected InitRoad(m2::Object& obj, const m2::VecF& position, Connection connection);
