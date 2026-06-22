#pragma once

#include <m2/thirdparty/video/Detail.h>

namespace m2 {
	// A timer for in-game time keeping. It doesn't get effected by UI pauses.
	// Timer is immutable, create a new object if you want to reset it.
	// TODO replace this with a special Stopwatch variant
	class Timer {
		thirdparty::video::Ticks _start_ticks;

	public:
		Timer();
		[[nodiscard]] thirdparty::video::Ticks ticks_since_start() const;
		[[nodiscard]] bool has_ticks_passed(thirdparty::video::Ticks ticks) const;
	};
}
