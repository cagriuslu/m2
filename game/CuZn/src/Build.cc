#include <cuzn/Build.h>
#include <cuzn/Detail.h>
#include <cuzn/object/Tile.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>

m2::expected<m2g::pb::ItemType> cuzn::can_player_build_industry(m2::Character& player, m2g::pb::SpriteType location, m2g::pb::ItemType industry) {
	// Ensure that the location contains the given industry
	auto industries = industries_on_location(location);
	if (std::find(industries.begin(), industries.end(), industry) == industries.end()) {
		throw M2ERROR("Location does not contain the industry");
	}

	// Check if the location already has a tile
	if (find_tile_at_location(location)) {
		return m2::make_unexpected("Location already has a tile built");
	}

	// Find the city of the location
	auto city = city_of_location(location);
	// Find all the locations in the city
	auto locations = locations_in_city(city);
	// Remove the given location
	locations.erase(std::remove(locations.begin(), locations.end(), location), locations.end());

	// Check if this location is part of the player's network, or they don't have a network.
	if (auto cities_in_network = get_cities_in_network(player); not cities_in_network.empty() && not cities_in_network.contains(city)) {
		return m2::make_unexpected("City is not in player's network");
	}

	// If there's more than one industry on this location, check if there's another location in the city with only this industry.
	if (1 < industries.size()) {
		// Gather locations that only has the industry
		std::vector<m2g::pb::SpriteType> locations_with_only_the_industry;
		std::copy_if(locations.begin(), locations.end(), std::back_inserter(locations_with_only_the_industry),
			[industry](m2g::pb::SpriteType loc) {
			auto industries = industries_on_location(loc);
			return (industries.size() == 1 && industries[0] == industry);
		});
		// If there's a location with only the given industry
		for (auto location_with_only_the_industry : locations_with_only_the_industry) {
			// Check if those locations are built already
			if (find_tile_at_location(location_with_only_the_industry)) {
				return m2::make_unexpected("Cannot build industry while the city has an empty location with only that industry");
			}
		}
	}

	// Check if it's the first era and the player already has a tile in the city
	if (player.get_resource(m2g::pb::ERA) == 1.0f) {
		// Iterate over other locations in the city
		for (const auto& other_location : locations) {
			// If the location has a tile built
			if (auto* tile_obj = find_tile_at_location(other_location); tile_obj) {
				// If the parent of the tile is the given player
				if (tile_obj->parent_id() == player.parent().id()) {
					return m2::make_unexpected("Cannot build multiple industries in same city in first era");
				}
			}
		}
	}

	// Check if the player has a tile to build
	auto tile_type = get_next_buildable_tile(player, industry_card_to_tile_category(industry));
	if (not tile_type) {
		return m2::make_unexpected("Player doesn't have tiles left of selected industry");
	}
	const auto& tile = GAME.get_named_item(*tile_type);

	// Check if the tile can be built in this era
	if (player.get_resource(m2g::pb::ERA) == tile.get_attribute(m2g::pb::FORBIDDEN_ERA)) {
		return m2::make_unexpected("The next tile of selected industry cannot be built in this era");
	}

	// Check if player has enough money
	if (player.get_resource(m2g::pb::MONEY) < tile.get_attribute(m2g::pb::MONEY_COST)) {
		return m2::make_unexpected("Player doesn't have enough money");
	}

	return *tile_type;
}
