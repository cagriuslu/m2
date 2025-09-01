#pragma once
#include <chrono>

namespace m2 {
	class Stopwatch {
		std::chrono::time_point<std::chrono::steady_clock> _startTimePoint;
	public:
		// Exposed types

		using Duration = std::chrono::steady_clock::duration;

		// Constants

		static constexpr Duration OneSecondInDuration() { return std::chrono::duration_cast<Duration>(std::chrono::seconds{1}); }

		// Constructors

		Stopwatch() : _startTimePoint(std::chrono::steady_clock::now()) {}

		[[nodiscard]] Duration GetDurationSince() const {
			return std::chrono::steady_clock::now() - _startTimePoint;
		}
		[[nodiscard]] std::chrono::milliseconds GetDurationSinceMs() const {
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _startTimePoint);
		}
		[[nodiscard]] std::chrono::microseconds GetDurationSinceUs() const {
			return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - _startTimePoint);
		}
		[[nodiscard]] float GetDurationSinceF() const {
			return std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - _startTimePoint).count();
		}

		[[nodiscard]] bool HasTimePassed(const Duration& d) const {
			return d <= GetDurationSince();
		}
		[[nodiscard]] bool HasTimePassed(const std::chrono::milliseconds ms) const {
			return ms <= GetDurationSinceMs();
		}
		[[nodiscard]] bool HasTimePassed(const std::chrono::microseconds us) const {
			return us <= GetDurationSinceUs();
		}

		/// Returns the current duration since start and resets the stopwatch.
		Duration Reset() {
			const auto now = std::chrono::steady_clock::now();
			const auto durationBeforeReset = now - _startTimePoint;
			_startTimePoint = now;
			return durationBeforeReset;
		}
		/// Advances the starting point of the stopwatch so that it behaves as if it was started later.
		void AdvanceStartingPoint(const Duration& duration) {
			_startTimePoint += duration;
		}
		/// Retract the starting point of the stopwatch so that it behaves as if it was started earlier.
		void RetractStartingPoint(const Duration& duration) {
			_startTimePoint -= duration;
		}
	};

	inline float ToDurationF(const Stopwatch::Duration& d) { return std::chrono::duration_cast<std::chrono::duration<float>>(d).count(); }
}
