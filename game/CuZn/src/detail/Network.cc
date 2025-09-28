#include <cuzn/detail/Network.h>
#include <cuzn/object/Road.h>
#include <cuzn/Detail.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>
#include <m2/Log.h>

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
			if (FindRoadAtLocation(conn)) {
				// Iterate over cities of connection
				for (auto conn_city : cities_from_connection(conn)) {
					if (is_merchant_city(conn_city)) {
						auto merchant_locations = merchant_locations_of_merchant_city(conn_city);
						locations.insert(merchant_locations.begin(), merchant_locations.end());
					} else if (is_industry_city(conn_city) && not processed_cities.contains(conn_city)) {
						recursive_location_network_from_industry_city(locations, processed_cities, conn_city);
					}
				}
			}
		}
	}
}

std::set<Location> ReachableLocationsFromIndustryCity(IndustryCity city) {
	if (not is_industry_city(city)) {
		throw M2_ERROR("Invalid industry city");
	}

	std::set<Location> locations;
	std::set<IndustryCity> processed_cities;
	recursive_location_network_from_industry_city(locations, processed_cities, city);
	return locations;
}

bool IsIndustryCityConnectedToLocation(IndustryCity city, Location location) {
	auto network = ReachableLocationsFromIndustryCity(city);
	return network.contains(location);
}

m2::Graph<City> CreateActiveConnectionsGraph() {
	m2::Graph<City> active_connections;
	for (const auto& road_chr : M2_LEVEL.characters | std::views::transform(m2::ToCharacterBase) | std::views::filter(IsRoadCharacter)) {
		// Get the cities connected by the road
		std::vector<City> cities;
		std::transform(road_chr.FindItems(ITEM_CATEGORY_CITY_CARD), road_chr.EndItems(), std::back_inserter(cities), [](const auto& item) { return item.Type(); });
		// Fill the graph
		if (cities.size() == 2) {
			active_connections.AddEdge({cities[0], cities[1], m2::Float::One()});
			active_connections.AddEdge({cities[1], cities[0], m2::Float::One()});
		} else if (cities.size() == 3) {
			active_connections.AddEdge({cities[0], cities[1], m2::Float::One()});
			active_connections.AddEdge({cities[1], cities[0], m2::Float::One()});
			active_connections.AddEdge({cities[0], cities[2], m2::Float::One()});
			active_connections.AddEdge({cities[2], cities[0], m2::Float::One()});
			active_connections.AddEdge({cities[1], cities[2], m2::Float::One()});
			active_connections.AddEdge({cities[2], cities[1], m2::Float::One()});
		} else {
			throw M2_ERROR("Invalid connection");
		}
	}
	return active_connections;
}
