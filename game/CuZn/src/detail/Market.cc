#include <cuzn/detail/Market.h>
#include <m2/M2.h>

int CalculateCost(int capacity, int current_resource_count, int to_buy) {
	auto cost_of_one = [&](int resource_count) {
		if (0 < resource_count) {
			return (capacity / 2) + 1 - m2::I(ceilf(m2::ToFloat(resource_count) / 2.0f));
		} else {
			return (capacity / 2) + 1;
		}
	};

	int total_cost = 0;
	m2Repeat(to_buy) {
		total_cost += cost_of_one(current_resource_count--);
	}
	return total_cost;
}

std::pair<int,int> CalculateRevenue(int capacity, int current_resource_count, int count) {
	auto revenue_of_one = [&](int resource_count) {
		return (capacity / 2) - m2::I(floorf(m2::ToFloat(resource_count) / 2.0f));
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
