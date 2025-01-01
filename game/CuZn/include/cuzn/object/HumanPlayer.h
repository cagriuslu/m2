#pragma once
#include <m2/component/Character.h>
#include <m2/Object.h>
#include <m2/Meta.h>
#include <set>
#include <cuzn/Detail.h>

m2::void_expected InitThisHumanPlayerInstance(m2::Object& obj);
m2::void_expected InitOtherHumanPlayerInstance(m2::Object& obj);

// Accessors
size_t PlayerCardCount(m2::Character& player);
std::list<Card> PlayerCards(m2::Character& player);
inline bool DoesPlayerHoldCard(m2::Character& player, Card card) { return player.find_items(card) != player.end_items(); }
int PlayerLinkCount(m2::Character& player);
int PlayerVictoryPoints(m2::Character& player);
int PlayerIncomePoints(m2::Character& player);
int PlayerMoney(m2::Character& player);
size_t player_tile_count(m2::Character& player);
inline size_t player_available_road_count(m2::Character& player) { return player.count_item(m2g::pb::ROAD_TILE); }
std::optional<m2g::pb::ItemType> get_next_industry_tile_of_category(m2::Character& player, m2g::pb::ItemCategory tile_category);
std::optional<m2g::pb::ItemType> get_next_industry_tile_of_industry(m2::Character& player, Industry);
/// Returns the number of industries belonging to the player
size_t player_built_factory_count(m2::Character& player);
/// Returns the locations of the industries belonging to the player
std::set<IndustryLocation> player_built_factory_locations(m2::Character& player);
/// Returns the locations of the industries of type COTTON, POTTERY, and MANUFACTURED_GOODS belonging to the player
std::set<IndustryLocation> player_sellable_factory_locations(m2::Character& player);

m2::void_expected can_player_overbuild_on_location_with_card(const m2::Character& player, IndustryLocation location, Card card);

std::set<m2g::pb::ItemType> get_cities_in_network(m2::Character& player);
std::set<m2g::pb::SpriteType> get_canals_in_network(m2::Character& player, Connection provisional_extra_connection = {});
std::set<m2g::pb::SpriteType> get_railroads_in_network(m2::Character& player, Connection provisional_extra_connection = {});
std::set<m2g::pb::SpriteType> get_connections_in_network(m2::Character& player, Connection provisional_extra_connection = {});
