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
	auto active_connections = CreateActiveConnectionsGraph();
	// Find all reachable cities and their costs
	auto reachable_cities = active_connections.FindNodesReachableFrom(city, m2::Float{100.0f});
	if (city_2) {
		auto reachable_cities_2 = active_connections.FindNodesReachableFrom(city_2, m2::Float{100.0f});
		reachable_cities = m2::Graph<City>::merge_reachable_nodes(reachable_cities, reachable_cities_2);
	}

	// Order cities by cost
	auto cities_ordered_by_cost = m2::Graph<City>::OrderByBestCost(reachable_cities);
	// Find the closest coal mines with the same lowest cost
	std::set<IndustryLocation> industry_locations;
	m2::Float closest_coal_mine_cost{};
	for (const auto& [cost, node] : cities_ordered_by_cost) {
		if (not industry_locations.empty() && closest_coal_mine_cost.IsLess(cost, m2::Float{0.001f})) {
			// If some coal mines are found and there aren't any other cities as close, stop searching.
			break;
		}

		// Check if the city contains a coal resource
		auto locs_in_city = industry_locations_in_city(static_cast<City>(node));
		for (const auto& loc : locs_in_city) {
			if (auto* factory = FindFactoryAtLocation(loc); factory && factory->GetCharacter().HasResource(COAL_CUBE_COUNT)) {
				industry_locations.emplace(loc);
				closest_coal_mine_cost = cost;
			}
		}

	}
	return industry_locations;
}

std::optional<MerchantCity> find_connected_coal_market(City city, City city_2) {
	auto active_connections = CreateActiveConnectionsGraph();
	// Find all reachable cities and their costs
	auto reachable_cities = active_connections.FindNodesReachableFrom(city, m2::Float{100.0f});
	// Check if any of the reachable cities is a merchant city
	for (const auto& [node, cost] : reachable_cities) {
		if (is_merchant_city(static_cast<City>(node))) {
			return static_cast<City>(node);
		}
	}

	if (city_2) {
		// Find all reachable cities and their costs
		auto reachable_cities_2 = active_connections.FindNodesReachableFrom(city_2, m2::Float{100.0f});
		// Check if any of the reachable cities is a merchant city
		for (const auto& [node, cost] : reachable_cities_2) {
			if (is_merchant_city(static_cast<City>(node))) {
				return static_cast<City>(node);
			}
		}
	}

	return std::nullopt;
}

bool is_there_coal_on_the_board() {
	// Iterate over factories
	auto there_is_coal_mine_with_coal = std::ranges::any_of(
			M2_LEVEL.characters
				| std::views::transform(m2::ToCharacterBase)
				| std::views::filter(IsFactoryCharacter),
			[](m2::Character& chr) {
				return m2::IsPositive(chr.GetResource(COAL_CUBE_COUNT), 0.001f);
			});
	// Check the market as well
	return there_is_coal_mine_with_coal
		|| m2::IsPositive(M2G_PROXY.game_state_tracker().GetResource(COAL_CUBE_COUNT), 0.001f);
}
