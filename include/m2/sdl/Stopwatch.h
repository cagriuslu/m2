#pragma once
#include "Detail.h"

namespace m2::sdl {
	class Stopwatch {
		ticks_t _total_ticks{};
		ticks_t _lap{};

	public:
		ticks_t measure(ticks_t pause_ticks) {
			// Measure how much time has passed since last reset lap
			ticks_t measurement = get_ticks_since(_total_ticks, pause_ticks);
			_total_ticks += measurement;
			return (_lap += measurement);
		}
		ticks_t subtract_from_lap(ticks_t t) {
			return (_lap = (_lap < t) ? 0 : _lap - t);
		}
		void new_lap() { _lap = 0; }
		[[nodiscard]] ticks_t lap() const { return _lap; }
	};
}
