#pragma once
#include <m2g_CardType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/component/character/FastCharacter.h>
#include <m2/component/Character.h>
#include <m2/math/VecF.h>
#include <m2/M2.h>

using City = m2g::pb::CardType;
using IndustryCity = City;
using MerchantCity = City;
using Industry = m2g::pb::CardType;
using SellableIndustry = m2g::pb::CardType;
using IndustryTile = m2g::pb::CardType;
using IndustryTileCategory = m2g::pb::CardCategory;
using IndustrySprite = m2g::pb::SpriteType;
using Location = m2g::pb::SpriteType;
using IndustryLocation = Location;
using MerchantLocation = Location;
using Connection = m2g::pb::SpriteType;
using POI = m2g::pb::SpriteType; // Location or Connection

// Characters

m2::Pool<m2::FastCharacter>& GetCharacterPool();

// Assertions

bool is_card(m2g::pb::CardType card);
bool is_city(City city);
bool is_industry_city(IndustryCity city);
bool is_merchant_city(MerchantCity city);
bool is_industry(Industry industry);
bool is_sellable_industry(Industry industry);
bool is_industry_tile(IndustryTile industry_tile);
bool is_industry_tile_category(IndustryTileCategory category);
bool is_industry_sprite(IndustrySprite industry_sprite);
bool is_location(Location location);
bool is_industry_location(IndustryLocation location);
bool is_merchant_location(MerchantLocation location);
bool DoesMerchantHasDevelopBenefit(MerchantLocation location);
int MerchantIncomePointsBenefit(MerchantLocation location);
int MerchantVictoryPointsBenefit(MerchantLocation location);
int MerchantMoneyBenefit(MerchantLocation location);
bool is_connection(Connection connection);
bool is_canal(Connection connection);
bool is_railroad(Connection connection);
bool is_poi(POI lc);
bool is_canal_license(m2g::pb::CardType card);
bool is_railroad_license(m2g::pb::CardType card);
bool location_has_industry(IndustryLocation location, Industry industry);

// Converters

City city_of_location(Location location);
Industry industry_of_industry_tile(IndustryTile industry_tile);
Industry industry_of_industry_tile_category(IndustryTileCategory category);
IndustryTileCategory industry_tile_category_of_industry(Industry industry_card);
IndustryTileCategory industry_tile_category_of_industry_tile(IndustryTile industry_tile);
IndustrySprite industry_sprite_of_industry(Industry industry);
m2g::pb::CardType merchant_license_type_of_sellable_industry(SellableIndustry sellable_industry);
std::vector<MerchantLocation> merchant_locations_of_merchant_city(MerchantCity city);

// Generators

std::set<IndustryLocation> all_industry_locations();
std::set<Connection> all_canals();
std::set<Connection> all_railroads();

/// Returns the industry locations in that given city
std::vector<IndustryLocation> industry_locations_in_city(City city_card);

std::vector<Connection> connections_from_city(City city_card);

std::vector<City> cities_from_connection(Connection connection);
// Only major cities of a connection is returned. Excludes STANDALONE_LOCATION_2_CARD but includes STANDALONE_LOCATION_1_CARD
std::array<City, 2> major_cities_from_connection(Connection connection);

std::vector<Industry> industries_on_location(IndustryLocation location);

m2::VecF connection_sprite_world_offset(m2g::pb::SpriteType original_type);

// Locators

std::optional<IndustryLocation> industry_location_on_position(const m2::VecF& world_position);
std::optional<MerchantLocation> merchant_location_on_position(const m2::VecF& world_position);
m2::VecF position_of_industry_location(IndustryLocation industry_location);
std::optional<Connection> connection_on_position(const m2::VecF& world_position);
m2::VecF position_of_connection(Connection connection);
