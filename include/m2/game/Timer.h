#pragma once

#include "../sdl/Detail.hh"

namespace m2 {
	// A timer for in-game time keeping. It doesn't get effected by UI pauses.
	// Timer is immutable, create a new object if you want to reset it.
	class Timer {
		sdl::ticks_t _start_ticks;

	public:
		Timer();
		sdl::ticks_t ticks_since_start() const;
		bool has_ticks_passed(sdl::ticks_t ticks) const;
	};
}