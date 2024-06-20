#include <cuzn/detail/Network.h>
#include <cuzn/object/Road.h>
#include <cuzn/Detail.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>

using namespace m2g;
using namespace m2g::pb;

std::vector<std::pair<m2g::pb::ResourceType, float>> road_costs(bool double_railroad) {
	if (M2G_PROXY.is_canal_era()) {
		return {{MONEY, 3.0f}};
	} else if (!double_railroad) {
		return {{MONEY, 5.0f}, {COAL_CUBE_COUNT, 1.0f}};
	} else {
		return {{MONEY, 15.0f}, {COAL_CUBE_COUNT, 2.0f}, {BEER_BARREL_COUNT, 1.0f}};
	}
}

m2::expected<ItemType> can_player_build_connection(m2::Character& player, ItemType card, SpriteType location) {
	if (not player_has_card(player, card)) {
		return m2::make_unexpected("Player doesn't hold the given card");
	}
	if (find_road_at_location(location)) {
		return m2::make_unexpected("Location already has a road built");
	}

	if (M2G_PROXY.is_canal_era()) {
		if (not is_canal(location)) {
			return m2::make_unexpected("The location has no canal");
		}
	} else if (M2G_PROXY.is_railroad_era()) {
		if (not is_railroad(location)) {
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

namespace {
	void recursive_location_network_from_industry_city(std::set<Location>& locations, std::set<IndustryCity>& processed_cities, IndustryCity city) {
		// Add locations in the same city
		auto locs_in_city = industry_locations_in_city(city);
		for (auto loc : locs_in_city) {
			locations.emplace(loc);
		}
		processed_cities.emplace(city);

		// Iterate over connections
		for (auto conn : connections_from_city(city)) {
			// Only if the connection is built
			if (find_road_at_location(conn)) {
				// Iterate over cities of connection
				for (auto conn_city : cities_from_connection(conn)) {
					if (is_merchant_city(conn_city)) {
						locations.emplace(merchant_location_of_merchant_city(conn_city));
					} else if (is_industry_city(conn_city) && not processed_cities.contains(conn_city)) {
						recursive_location_network_from_industry_city(locations, processed_cities, city);
					}
				}
			}
		}
	}
}

std::set<Location> location_network_from_industry_city(IndustryCity city) {
	if (not is_industry_city(city)) {
		throw M2ERROR("Invalid industry city");
	}

	std::set<Location> locations;
	std::set<IndustryCity> processed_cities;
	recursive_location_network_from_industry_city(locations, processed_cities, city);
	return locations;
}

bool is_industry_city_connected_to_location(IndustryCity city, Location location) {
	auto network = location_network_from_industry_city(city);
	return network.contains(location);
}
