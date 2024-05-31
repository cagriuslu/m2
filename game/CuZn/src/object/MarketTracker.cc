#include <cuzn/object/MarketTracker.h>

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

	chr.on_interaction = [](m2::Character& self, MAYBE m2::Character* other, const m2g::pb::InteractionData& data) -> std::optional<m2g::pb::InteractionData> {
		if (data.has_ask_coal_cost()) {
			auto ask_count = data.ask_coal_cost();
			auto current_coal_count = m2::iround(self.get_resource(m2g::pb::COAL_CUBE_COUNT));
			auto cost = calculate_cost(COAL_MARKET_CAPACITY, current_coal_count, ask_count);

			m2g::pb::InteractionData response;
			response.set_return_coal_cost(cost);
			return response;
		} else if (data.has_ask_iron_cost()) {
			auto ask_count = data.ask_iron_cost();
			auto current_iron_count = m2::iround(self.get_resource(m2g::pb::IRON_CUBE_COUNT));
			auto cost = calculate_cost(IRON_MARKET_CAPACITY, current_iron_count, ask_count);

			m2g::pb::InteractionData response;
			response.set_return_iron_cost(cost);
			return response;
		}
		return std::nullopt;
	};
}
