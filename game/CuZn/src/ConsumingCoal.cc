#include <cuzn/ConsumingCoal.h>
#include <m2/game/Graph.h>
#include <m2/Game.h>
#include <cuzn/detail/Network.h>
#include <cuzn/object/Road.h>
#include <cuzn/object/Factory.h>
#include <cuzn/Detail.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

std::set<IndustryLocation> find_closest_connected_coal_mines_with_coal(City city, City city_2) {
	auto active_connections = create_active_connections_graph();
	// Find all reachable cities and their costs
	auto reachable_cities = active_connections.reachable_nodes_from(city, 100.0f);
	if (city_2) {
		auto reachable_cities_2 = active_connections.reachable_nodes_from(city_2, 100.0f);
		reachable_cities = m2::Graph::merge_reachable_nodes(reachable_cities, reachable_cities_2);
	}

	// Order cities by cost
	auto cities_ordered_by_cost = m2::Graph::order_by_cost(reachable_cities);
	// Find the closest coal mines with the same lowest cost
	std::set<IndustryLocation> industry_locations;
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
				industry_locations.emplace(loc);
				closest_coal_mine_cost = cost;
			}
		}

	}
	return industry_locations;
}

std::optional<MerchantCity> find_connected_coal_market(City city, City city_2) {
	auto active_connections = create_active_connections_graph();
	// Find all reachable cities and their costs
	auto reachable_cities = active_connections.reachable_nodes_from(city, 100.0f);
	// Check if any of the reachable cities is a merchant city
	for (const auto& [node, cost] : reachable_cities) {
		if (is_merchant_city(static_cast<City>(node))) {
			return static_cast<City>(node);
		}
	}

	if (city_2) {
		// Find all reachable cities and their costs
		auto reachable_cities_2 = active_connections.reachable_nodes_from(city_2, 100.0f);
		// Check if any of the reachable cities is a merchant city
		for (const auto& [node, cost] : reachable_cities_2) {
			if (is_merchant_city(static_cast<City>(node))) {
				return static_cast<City>(node);
			}
		}
	}

	return std::nullopt;
}
