#include <m2/mt/WaitOnCondition.h>
#include <thread>

bool m2::mt::wait_on_condition(int duration_ms, const std::function<bool()>& condition, int test_interval) {
	int total_waited_ms = 0;
	while (total_waited_ms < duration_ms) {
		if (condition()) {
			return true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(test_interval));
	}
	return false;
}