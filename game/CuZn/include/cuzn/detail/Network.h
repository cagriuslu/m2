#pragma once
#include <m2/component/Character.h>
#include <m2g_ItemType.pb.h>
#include <m2g_SpriteType.pb.h>
#include <m2/M2.h>

namespace cuzn {
	std::vector<std::pair<m2g::pb::ResourceType, float>> road_costs(bool double_railroad = false);

	/// Validates whether player can build the given network in the given location using the given card.
	/// Returns the next buildable tile type from player's inventory.
	/// Coal and iron requirements of the tile is not validated.
	m2::expected<m2g::pb::ItemType> can_player_build_network(m2::Character& player, m2g::pb::ItemType card, m2g::pb::SpriteType location);
}
