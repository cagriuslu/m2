#pragma once
#include <m2g_ItemType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/component/Character.h>
#include <m2/VecF.h>
#include <m2/M2.h>

namespace cuzn {
	using Card = m2g::pb::ItemType;
	using Industry = m2g::pb::ItemType;
	using IndustryLocation = m2g::pb::SpriteType;
	using NetworkLocation = m2g::pb::SpriteType;

	bool is_card(Card card);
	bool is_industry(Industry industry);
	bool is_industry_location(IndustryLocation location);
	bool is_canal_location(NetworkLocation location);
	bool is_railroad_location(NetworkLocation location);
	bool is_canal_license(m2g::pb::ItemType item);
	bool is_railroad_license(m2g::pb::ItemType item);

	bool player_has_card(m2::Character& player, m2g::pb::ItemType card);

	/// Returns the city card for that given industry location sprite
	m2g::pb::ItemType city_of_location(m2g::pb::SpriteType location);

	/// Returns the industry locations in that given city
	std::vector<m2g::pb::SpriteType> locations_in_city(m2g::pb::ItemType city_card);

	/// Returns the industries in that given industry location
	std::vector<m2g::pb::ItemType> industries_on_location(m2g::pb::SpriteType location);
	bool location_has_industry(m2g::pb::SpriteType location, m2g::pb::ItemType industry);

	/// Convert an industry card representing an industry to an item category representing a tile type
	m2g::pb::ItemCategory industry_card_to_tile_category(m2g::pb::ItemType industry_card);

	/// Returns the industry located on the given position
	std::optional<m2g::pb::SpriteType> industry_location_on_position(const m2::VecF& world_position);

	/// Returns the network located on the given position
	std::optional<m2g::pb::SpriteType> network_location_on_position(const m2::VecF& world_position);
}
