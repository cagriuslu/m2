#pragma once
#include <m2/M2.h>
#include <m2/component/Character.h>

namespace cuzn {
	/// Returns the different industries in selected_location that can be built by the given selected_card
	std::vector<m2g::pb::ItemType> selectable_industries(m2g::pb::ItemType selected_card, m2g::pb::SpriteType selected_location);

	/// Validates whether player can build the given industry in the given location using the given card.
	/// Returns the next buildable tile type from player's inventory.
	/// Coal and iron requirements of the tile is not validated.
	m2::expected<m2g::pb::ItemType> can_player_build_industry(m2::Character& player, m2g::pb::ItemType card, m2g::pb::SpriteType location, m2g::pb::ItemType industry);
}
