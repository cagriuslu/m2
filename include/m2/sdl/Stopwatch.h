#pragma once
#include "Detail.h"

namespace m2::sdl {
	class Stopwatch {
		const ticks_t* _pause_ticks_ref;
		ticks_t _total_ticks{};
		ticks_t _lap{};
		ticks_t _last_measurement{};

	public:
		explicit Stopwatch(const ticks_t* pause_ticks_ref) : _pause_ticks_ref(pause_ticks_ref) {}
		ticks_t measure() {
			// Measure how much time has passed since last reset lap
			_last_measurement = get_ticks_since(_total_ticks, _pause_ticks_ref ? *_pause_ticks_ref : 0);
			_total_ticks += _last_measurement;
			return (_lap += _last_measurement);
		}
		ticks_t subtract_from_lap(ticks_t t) {
			return (_lap = (_lap < t) ? 0 : _lap - t);
		}
		void new_lap() { _lap = 0; }
		[[nodiscard]] ticks_t lap() const { return _lap; }
		[[nodiscard]] ticks_t last() const { return _last_measurement; }
	};
}
