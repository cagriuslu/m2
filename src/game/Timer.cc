#include <m2/game/Timer.h>
#include <m2/Game.h>

m2::Timer::Timer() : _start_ticks(m2::sdl::get_ticks() - M2_GAME.pause_ticks) {}

m2::sdl::ticks_t m2::Timer::ticks_since_start() const {
	return sdl::get_ticks_since(_start_ticks, M2_GAME.pause_ticks);
}

bool m2::Timer::has_ticks_passed(sdl::ticks_t ticks) const {
	return ticks < ticks_since_start();
}
