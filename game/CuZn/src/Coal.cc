#include <cuzn/Coal.h>
#include <m2/game/Graph.h>
#include <m2/Game.h>
#include <cuzn/object/Road.h>
#include <cuzn/object/Factory.h>
#include <cuzn/Detail.h>

using namespace m2g;
using namespace m2g::pb;

namespace {
	m2::Graph create_active_connections_graph() {
		m2::Graph active_connections;
		for (const auto& road_chr : M2_LEVEL.characters
									| std::views::transform(m2::to_character_base)
									| std::views::filter(is_road_character)) {
			// Get the cities connected by the road
			std::vector<City> cities;
			std::transform(road_chr.find_items(ITEM_CATEGORY_CITY_CARD), road_chr.end_items(), std::back_inserter(cities),
				[](const auto& item) {
					return item.type();
				});
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
}

std::vector<IndustryLocation> find_closest_connected_coal_mines_with_coal(City city) {
	auto active_connections = create_active_connections_graph();
	// Find all reachable cities and their costs
	auto reachable_cities = active_connections.reachable_nodes_from(city, 100.0f);
	// Order cities by cost
	auto cities_ordered_by_cost = m2::Graph::order_by_cost(reachable_cities);
	// Find the closest coal mines with the same lowest cost
	std::vector<IndustryLocation> industry_locations;
	float closest_coal_mine_cost = 0.0f;
	for (const auto& [cost, node] : cities_ordered_by_cost) {
		if (not industry_locations.empty() && m2::is_less(closest_coal_mine_cost, cost, 0.001)) {
			// If some coal mines are found and there aren't any other cities as close, stop searching.
			break;
		}

		// Check if the city contains a coal resource
		auto locs_in_city = industry_locations_in_city(static_cast<City>(node));
		for (const auto& loc : locs_in_city) {
			if (auto* factory = find_factory_at_location(loc); factory && factory->character().has_resource(COAL_CUBE_COUNT)) {
				industry_locations.emplace_back(loc);
				closest_coal_mine_cost = cost;
			}
		}

	}
	return industry_locations;
}

std::optional<MerchantCity> find_connected_coal_market_with_coal(City city) {
	auto active_connections = create_active_connections_graph();
	// Find all reachable cities and their costs
	auto reachable_cities = active_connections.reachable_nodes_from(city, 100.0f);
	// Check if any of the reachable cities is a merchant city
	for (const auto& [node, cost] : reachable_cities) {
		if (is_merchant_city(static_cast<City>(node))) {
			return static_cast<City>(node);
		}
	}
	return std::nullopt;
}
