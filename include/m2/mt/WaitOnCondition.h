#pragma once
#include <functional>

namespace m2::mt {
	// Waits until `condition` returns true for at most `duration_ms` milliseconds.
	// Returns false if `condition` never returns true. (timeout)
	// Returns true if `condition` returns true.
	bool wait_on_condition(int duration_ms, const std::function<bool()>& condition, int test_interval = 50);
}
