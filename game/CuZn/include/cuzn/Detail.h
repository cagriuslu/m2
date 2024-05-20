#pragma once
#include <m2g_ItemType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/component/Character.h>
#include <m2/VecF.h>
#include <m2/M2.h>

namespace cuzn {
	using Card = m2g::pb::ItemType;
	using City = m2g::pb::ItemType;
	using Industry = m2g::pb::ItemType;
	using IndustryTile = m2g::pb::ItemType;
	using IndustryTileCategory = m2g::pb::ItemCategory;
	using IndustrySprite = m2g::pb::SpriteType;
	using IndustryLocation = m2g::pb::SpriteType;
	using NetworkLocation = m2g::pb::SpriteType;

	// Assertions
	bool is_card(Card card);
	bool is_city(City city);
	bool is_industry(Industry industry);
	bool is_industry_tile(IndustryTile industry_tile);
	bool is_industry_tile_category(IndustryTileCategory category);
	bool is_industry_sprite(IndustrySprite industry_sprite);
	bool is_industry_location(IndustryLocation location);
	bool is_canal_location(NetworkLocation location);
	bool is_railroad_location(NetworkLocation location);
	bool is_canal_license(m2g::pb::ItemType item);
	bool is_railroad_license(m2g::pb::ItemType item);
	bool is_canal_era();
	bool is_railroad_era();

	// Converters
	City city_of_industry_location(IndustryLocation location);
	Industry industry_of_industry_tile(IndustryTile industry_tile);
	Industry industry_of_industry_tile_category(IndustryTileCategory category);
	IndustryTileCategory industry_tile_category_of_industry(Industry industry_card);
	IndustryTileCategory industry_tile_category_of_industry_tile(IndustryTile industry_tile);
	IndustrySprite industry_sprite_of_industry(Industry industry);

	/// Returns the industry locations in that given city
	std::vector<IndustryLocation> locations_in_city(City city_card);

	/// Returns the industries in that given industry location
	std::vector<Industry> industries_on_location(IndustryLocation location);
	bool location_has_industry(IndustryLocation location, Industry industry);

	// Locators
	std::optional<IndustryLocation> industry_location_on_position(const m2::VecF& world_position);
	m2::VecF position_of_industry_location(IndustryLocation industry_location);
	std::optional<NetworkLocation> network_location_on_position(const m2::VecF& world_position);

	SDL_Color generate_player_color(unsigned index);
}
