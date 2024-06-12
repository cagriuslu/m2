#include <cuzn/object/MarketTracker.h>
#include "m2/Game.h"

namespace {
	constexpr int COAL_MARKET_CAPACITY = 14;
	constexpr int IRON_MARKET_CAPACITY = 10;
	constexpr int COAL_MARKET_INITIAL_COUNT = 13;
	constexpr int IRON_MARKET_INITIAL_COUNT = 8;

	// Returns the cost of buying `to_buy` number of items from the market
	int calculate_cost(int capacity, int current_resource_count, int to_buy) {
		auto cost_of_one = [&](int resource_count) {
			if (0 < resource_count) {
				return (capacity / 2) + 1 - m2::I(ceilf(m2::F(resource_count) / 2.0f));
			} else {
				return (capacity / 2) + 1;
			}
		};

		int total_cost = 0;
		m2_repeat(to_buy) {
			total_cost += cost_of_one(current_resource_count--);
		}
		return total_cost;
	}

	// Returns the number of items that can be solved, and the total revenue of doing so.
	std::pair<int,int> calculate_revenue(int capacity, int current_resource_count, int count) {
		auto revenue_of_one = [&](int resource_count) {
			return (capacity / 2) - m2::I(floorf(m2::F(resource_count) / 2.0f));
		};

		int sold = 0;
		int total_revenue = 0;
		while (0 < count && 0 < (capacity - current_resource_count)) {
			total_revenue += revenue_of_one(current_resource_count);
			--count;
			++current_resource_count;
			++sold;
		}
		return std::make_pair(sold, total_revenue);
	}
}

void init_market(m2::Object& obj) {
	auto& chr = obj.add_full_character();
	chr.set_resource(m2g::pb::COAL_CUBE_COUNT, COAL_MARKET_INITIAL_COUNT);
	chr.set_resource(m2g::pb::IRON_CUBE_COUNT, IRON_MARKET_INITIAL_COUNT);
}

int market_coal_cost(int coal_count) {
	auto current_coal_count = m2::iround(M2G_PROXY.market_character().get_resource(m2g::pb::COAL_CUBE_COUNT));
	return calculate_cost(COAL_MARKET_CAPACITY, current_coal_count, coal_count);
}

int market_iron_cost(int iron_count) {
	auto current_iron_count = m2::iround(M2G_PROXY.market_character().get_resource(m2g::pb::IRON_CUBE_COUNT));
	return calculate_cost(IRON_MARKET_CAPACITY, current_iron_count, iron_count);
}

std::pair<int,int> market_coal_revenue(int count) {
	auto current_coal_count = m2::iround(M2G_PROXY.market_character().get_resource(m2g::pb::COAL_CUBE_COUNT));
	return calculate_revenue(COAL_MARKET_CAPACITY, current_coal_count, count);
}

std::pair<int,int> market_iron_revenue(int count) {
	auto current_iron_count = m2::iround(M2G_PROXY.market_character().get_resource(m2g::pb::IRON_CUBE_COUNT));
	return calculate_revenue(IRON_MARKET_CAPACITY, current_iron_count, count);
}
