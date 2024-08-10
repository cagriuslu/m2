#pragma once
#include <m2g_ItemType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/component/Character.h>
#include <m2/VecF.h>
#include <m2/M2.h>
#include <m2/Color.h>

using Card = m2g::pb::ItemType;
using City = m2g::pb::ItemType;
using IndustryCity = City;
using MerchantCity = City;
using Industry = m2g::pb::ItemType;
using IndustryTile = m2g::pb::ItemType;
using IndustryTileCategory = m2g::pb::ItemCategory;
using IndustrySprite = m2g::pb::SpriteType;
using Location = m2g::pb::SpriteType;
using IndustryLocation = Location;
using MerchantLocation = Location;
using Connection = m2g::pb::SpriteType;

// Assertions

bool is_card(Card card);
bool is_city(City city);
bool is_industry_city(IndustryCity city);
bool is_merchant_city(MerchantCity city);
bool is_industry(Industry industry);
bool is_industry_tile(IndustryTile industry_tile);
bool is_industry_tile_category(IndustryTileCategory category);
bool is_industry_sprite(IndustrySprite industry_sprite);
bool is_location(Location location);
bool is_industry_location(IndustryLocation location);
bool is_merchant_location(MerchantLocation location);
bool is_connection(Connection connection);
bool is_canal(Connection connection);
bool is_railroad(Connection connection);
bool is_canal_license(m2g::pb::ItemType item);
bool is_railroad_license(m2g::pb::ItemType item);
bool location_has_industry(IndustryLocation location, Industry industry);

// Converters

City city_of_location(Location location);
Industry industry_of_industry_tile(IndustryTile industry_tile);
Industry industry_of_industry_tile_category(IndustryTileCategory category);
IndustryTileCategory industry_tile_category_of_industry(Industry industry_card);
IndustryTileCategory industry_tile_category_of_industry_tile(IndustryTile industry_tile);
IndustrySprite industry_sprite_of_industry(Industry industry);
std::vector<MerchantLocation> merchant_locations_of_merchant_city(MerchantCity city); // TODO one city might have more than one merchant location

// Generators

std::set<IndustryLocation> all_industry_locations();
std::set<Connection> all_canals();
std::set<Connection> all_railroads();

/// Returns the industry locations in that given city
std::vector<IndustryLocation> industry_locations_in_city(City city_card);

std::vector<Connection> connections_from_city(City city_card);

std::vector<City> cities_from_connection(Connection connection);

std::vector<Industry> industries_on_location(IndustryLocation location);

m2::VecF connection_sprite_world_offset(m2g::pb::SpriteType original_type);

// Locators

std::optional<IndustryLocation> industry_location_on_position(const m2::VecF& world_position);
std::optional<MerchantLocation> merchant_location_on_position(const m2::VecF& world_position);
m2::VecF position_of_industry_location(IndustryLocation industry_location);
std::optional<Connection> connection_on_position(const m2::VecF& world_position);
m2::VecF position_of_connection(Connection connection);
