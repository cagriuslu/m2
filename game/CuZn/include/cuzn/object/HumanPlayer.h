#pragma once
#include <m2/component/Character.h>
#include <m2/Object.h>
#include <m2/Meta.h>
#include <set>

m2::void_expected init_human_player(m2::Object& obj);

// Accessors
size_t player_card_count(m2::Character& player);
bool player_has_card(m2::Character& player, m2g::pb::ItemType card);
size_t player_road_count(m2::Character& player);
int player_link_count(m2::Character& player);
float player_money(m2::Character& player);
size_t player_tile_count(m2::Character& player);
std::optional<m2g::pb::ItemType> get_next_buildable_factory(m2::Character& player, m2g::pb::ItemCategory tile_category);
size_t player_built_factory_count(m2::Character& player);
std::set<m2g::pb::ItemType> get_cities_in_network(m2::Character& player);
std::set<m2g::pb::SpriteType> get_canals_in_network(m2::Character& player);
std::set<m2g::pb::SpriteType> get_railroads_in_network(m2::Character& player);
