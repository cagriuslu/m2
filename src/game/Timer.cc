#include <m2/game/Timer.h>
#include <m2/Game.h>

m2::Timer::Timer() : _start_ticks(m2::sdl::get_ticks() - std::chrono::duration_cast<std::chrono::milliseconds>(M2_LEVEL.GetTotalPauseDuration()).count()) {}

m2::sdl::ticks_t m2::Timer::ticks_since_start() const {
	return sdl::get_ticks_since(_start_ticks, std::chrono::duration_cast<std::chrono::milliseconds>(M2_LEVEL.GetTotalPauseDuration()).count());
}

bool m2::Timer::has_ticks_passed(sdl::ticks_t ticks) const {
	return ticks < ticks_since_start();
}
