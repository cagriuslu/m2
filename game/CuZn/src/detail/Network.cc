#include <cuzn/detail/Network.h>
#include <cuzn/object/Road.h>
#include <cuzn/Detail.h>
#include <cuzn/object/HumanPlayer.h>

using namespace m2g;
using namespace m2g::pb;

m2::expected<ItemType> cuzn::can_player_build_network(m2::Character& player, ItemType card, SpriteType location) {
	if (not player_has_card(player, card)) {
		return m2::make_unexpected("Player doesn't hold the given card");
	}
	if (find_road_at_location(location)) {
		return m2::make_unexpected("Location already has a road built");
	}

	auto era = player.get_resource(ERA);
	if (era == 1.0f) {
		if (not is_canal_location(location)) {
			return m2::make_unexpected("The location has no canal");
		}
	} else if (era == 2.0f) {
		if (not is_railroad_location(location)) {
			return m2::make_unexpected("The location has no railroad");
		}
	} else {
		throw M2ERROR("Invalid era");
	}

	// Check if this location is part of the player's network, or they don't have a network
	if (auto canals_in_network = get_canals_in_network(player); not canals_in_network.empty() && not canals_in_network.contains(location)) {
		return m2::make_unexpected("Location is not in player's network");
	}

	// Check if the player has a road to build
	// if (not player_has_road(player)) {
	// 	return m2::make_unexpected("Player doesn't have road tiles left");
	// }

	// Check if player has enough money

}
