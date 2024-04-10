#pragma once
#include <m2g_ItemType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/component/Character.h>
#include <m2/VecF.h>
#include <m2/M2.h>

namespace cuzn {
	bool is_card(m2g::pb::ItemType item);
	bool is_industry_location(m2g::pb::SpriteType sprite);

	/// Returns the city card for that given industry location sprite
	m2g::pb::ItemType city_of_location(m2g::pb::SpriteType location);

	/// Returns the industry locations in that given city
	std::vector<m2g::pb::SpriteType> locations_in_city(m2g::pb::ItemType city_card);

	/// Returns the industries in that given industry location
	std::vector<m2g::pb::ItemType> industries_on_location(m2g::pb::SpriteType location);

	/// Convert an industry card representing an industry to an item category representing a tile type
	m2g::pb::ItemCategory industry_card_to_tile_category(m2g::pb::ItemType industry_card);

	/// Returns the industry located on the given position
	std::optional<m2g::pb::SpriteType> industry_location_on_position(const m2::VecF& world_position);
}
