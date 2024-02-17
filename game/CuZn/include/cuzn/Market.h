#pragma once
#include <m2/Object.h>

namespace cuzn {
	constexpr int COAL_MARKET_CAPACITY = 14;
	constexpr int IRON_MARKET_CAPACITY = 10;
	constexpr int COAL_MARKET_INITIAL_COUNT = 13;
	constexpr int IRON_MARKET_INITIAL_COUNT = 8;

	void set_market_object_resource_count(m2::Id object_id, m2g::pb::ResourceType resource_type, int resource_count);

	// Host can use this class to keep track of the market status.
	// If the market transactions are made through this class, the Market object is also updated.
	// Clients can use a temporary instance of this class to calculate the cost and revenue of transactions.
	template <int Capacity>
	class Market {
		int _resource_count;
		m2g::pb::ResourceType _resource_type;
		m2::Id _market_object_id;

	   public:
		explicit Market(int initial_resource_count, m2g::pb::ResourceType resource_type, m2::Id market_object_id = 0)
		    : _resource_count(initial_resource_count),
		      _resource_type(resource_type),
		      _market_object_id(market_object_id) {}

		[[nodiscard]] int resource_count() const { return _resource_count; }
		[[nodiscard]] int resource_demand_count() const { return Capacity - _resource_count; }

		[[nodiscard]] int calculate_cost(int count) const {
			auto cost_of_one = [&](int resource_count) {
				if (0 < resource_count) {
					return (Capacity / 2) + 1 - m2::I(ceilf(m2::F(resource_count) / 2.0f));
				} else {
					return (Capacity / 2) + 1;
				}
			};

			int resource_count = _resource_count;
			int total_cost = 0;
			m2_repeat(count) { total_cost += cost_of_one(resource_count--); }
			return total_cost;
		}
		/// Returns -1 is there isn't enough demand
		[[nodiscard]] int calculate_revenue(int count) const {
			auto revenue_of_one = [&](int resource_count) {
				return (Capacity / 2) - m2::I(floorf(m2::F(resource_count) / 2.0f));
			};

			if (count <= resource_demand_count()) {
				int resource_count = _resource_count;
				int total_revenue = 0;
				m2_repeat(count) { total_revenue += revenue_of_one(resource_count++); }
				return total_revenue;
			} else {
				return -1;
			}
		}

		/// Returns total cost
		int buy_resources(int count) {
			auto cost = calculate_cost(count);
			_resource_count = std::max(0, _resource_count - count);

			if (_market_object_id) {
				// Set Market object resource so that the clients can receive it
				set_market_object_resource_count(_market_object_id, _resource_type, _resource_count);
			}

			return cost;
		}
		/// Returns -1 is there isn't enough demand, otherwise returns total revenue
		int sell_resources(int count) {
			auto revenue = calculate_revenue(count);
			if (revenue == -1) {
				return -1;
			}
			_resource_count = std::min(Capacity, _resource_count + count);

			if (_market_object_id) {
				// Set Market object resource so that the clients can receive it
				set_market_object_resource_count(_market_object_id, _resource_type, _resource_count);
			}

			return revenue;
		}
	};
}  // namespace cuzn
