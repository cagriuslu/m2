#pragma once
#include <m2/component/Character.h>
#include <m2/Object.h>
#include <m2/Meta.h>
#include <set>
#include <cuzn/Detail.h>

m2::void_expected init_human_player(m2::Object& obj);

// Accessors
size_t player_card_count(m2::Character& player);
std::list<Card> player_cards(m2::Character& player);
int player_link_count(m2::Character& player);
int player_income_points(m2::Character& player);
size_t player_tile_count(m2::Character& player);
std::optional<m2g::pb::ItemType> get_next_buildable_industry_tile(m2::Character& player, m2g::pb::ItemCategory tile_category);
size_t player_built_factory_count(m2::Character& player);
std::set<IndustryLocation> player_built_factory_locations(m2::Character& player);

/// Returns the cities that fall into the network of the given player
std::set<m2g::pb::ItemType> get_cities_in_network(m2::Character& player);

/// Returns the canals that fall into the network of the given player
std::set<m2g::pb::SpriteType> get_canals_in_network(m2::Character& player);

/// Returns the railroads that fall into the network of the given player
std::set<m2g::pb::SpriteType> get_railroads_in_network(m2::Character& player);
