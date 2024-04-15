#pragma once
#include <m2/component/Character.h>
#include <m2/Object.h>
#include <m2/Meta.h>
#include <set>

namespace cuzn {
	m2::void_expected init_human_player(m2::Object& obj);

	std::optional<m2g::pb::ItemType> get_next_buildable_tile(m2::Character& player, m2g::pb::ItemCategory tile_category);
	std::set<m2g::pb::ItemType> get_cities_in_network(m2::Character& player);
	std::set<m2g::pb::SpriteType> get_canals_in_network(m2::Character& player);
	std::set<m2g::pb::SpriteType> get_railroads_in_network(m2::Character& player);
}
