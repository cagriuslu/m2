#include <cuzn/detail/Income.h>
#include <m2/Log.h>

namespace {
	std::vector<int> income_point_level_points = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
												  3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
												  4, 4, 4, 4, 4, 4, 4, 4, 4, 3};
}

int income_level_from_income_points(int ip) {
	if (ip < -10 || 89 < ip) {
		throw M2_ERROR("Invalid income points");
	}

	if (ip <= 0) {
		return ip;
	}

	for (size_t level_minus_one = 0; level_minus_one < income_point_level_points.size(); ++level_minus_one) {
		ip -= income_point_level_points[level_minus_one];
		if (ip <= 0) {
			return static_cast<int>(level_minus_one) + 1;
		}
	}
	throw M2_ERROR("Invalid income_point_level_points map");
}

int highest_income_points_of_level(const int level) {
	if (level < -10 || 30 < level) {
		throw M2_ERROR("Invalid income level");
	}

	if (level <= 0) {
		return level;
	} else {
		int ip = 0;
		for (int i = 0; i < level; ++i) {
			ip += income_point_level_points[i];
		}
		return ip;
	}
}
