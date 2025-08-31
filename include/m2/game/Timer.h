#pragma once

#include "../sdl/Detail.h"

namespace m2 {
	// A timer for in-game time keeping. It doesn't get effected by UI pauses.
	// Timer is immutable, create a new object if you want to reset it.
	// TODO replace this with a special Stopwatch variant
	class Timer {
		sdl::ticks_t _start_ticks;

	public:
		Timer();
		[[nodiscard]] sdl::ticks_t ticks_since_start() const;
		[[nodiscard]] bool has_ticks_passed(sdl::ticks_t ticks) const;
	};
}
