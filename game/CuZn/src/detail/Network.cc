#include <cuzn/detail/Network.h>
#include <cuzn/object/Road.h>
#include <cuzn/Detail.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>

using namespace m2g;
using namespace m2g::pb;

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
						auto merchant_locations = merchant_locations_of_merchant_city(conn_city);
						locations.insert(merchant_locations.begin(), merchant_locations.end());
					} else if (is_industry_city(conn_city) && not processed_cities.contains(conn_city)) {
						recursive_location_network_from_industry_city(locations, processed_cities, city);
					}
				}
			}
		}
	}
}

std::set<Location> reachable_locations_from_industry_city(IndustryCity city) {
	if (not is_industry_city(city)) {
		throw M2_ERROR("Invalid industry city");
	}

	std::set<Location> locations;
	std::set<IndustryCity> processed_cities;
	recursive_location_network_from_industry_city(locations, processed_cities, city);
	return locations;
}

bool is_industry_city_connected_to_location(IndustryCity city, Location location) {
	auto network = reachable_locations_from_industry_city(city);
	return network.contains(location);
}

m2::Graph create_active_connections_graph() {
	m2::Graph active_connections;
	for (const auto& road_chr : M2_LEVEL.characters
								| std::views::transform(m2::to_character_base)
								| std::views::filter(is_road_character)) {
		// Get the cities connected by the road
		std::vector<City> cities;
		std::transform(road_chr.find_items(ITEM_CATEGORY_CITY_CARD), road_chr.end_items(), std::back_inserter(cities),
			[](const auto& item) { return item.type(); });
		// Fill the graph
		if (cities.size() == 2) {
			active_connections.add_edge(cities[0], {cities[1], 1.0f});
			active_connections.add_edge(cities[1], {cities[0], 1.0f});
		} else if (cities.size() == 3) {
			active_connections.add_edge(cities[0], {cities[1], 1.0f});
			active_connections.add_edge(cities[1], {cities[0], 1.0f});
			active_connections.add_edge(cities[0], {cities[2], 1.0f});
			active_connections.add_edge(cities[2], {cities[0], 1.0f});
			active_connections.add_edge(cities[1], {cities[2], 1.0f});
			active_connections.add_edge(cities[2], {cities[1], 1.0f});
		} else {
			throw M2_ERROR("Invalid connection");
		}
	}
	return active_connections;
}
