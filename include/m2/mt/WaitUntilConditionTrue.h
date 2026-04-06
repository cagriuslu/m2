#pragma once
#include <thread>

namespace m2 {
	// Waits until `condition` returns true for at most `durationMs` milliseconds.
	// Returns false if `condition` never returns true. (timeout)
	// Returns true if `condition` returns true.
	template <typename Condition>
	bool WaitUntilConditionTrue(const int durationMs, Condition condition, const int testIntervalMs = 50) {
		int totalWaitedMs = 0;
		while (totalWaitedMs < durationMs) {
			if (condition()) { return true; }
			std::this_thread::sleep_for(std::chrono::milliseconds(testIntervalMs));
			totalWaitedMs += testIntervalMs;
		}
		return false;
	}
}
