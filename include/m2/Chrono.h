#pragma once
#include <chrono>

namespace m2 {
	class Stopwatch {
		std::chrono::time_point<std::chrono::steady_clock> _startTimePoint;
	public:
		Stopwatch() : _startTimePoint(std::chrono::steady_clock::now()) {}

		[[nodiscard]] std::chrono::milliseconds GetDurationSinceMs() const {
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _startTimePoint);
		}
	};
}
