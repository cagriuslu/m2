#include <m2/game/Timer.h>
#include <m2/Game.h>

m2::Timer::Timer() : _start_ticks(m2::thirdparty::video::GetTicks() - std::chrono::duration_cast<std::chrono::milliseconds>(M2_LEVEL.GetTotalPauseDuration()).count()) {}

m2::thirdparty::video::Ticks m2::Timer::ticks_since_start() const {
	return thirdparty::video::GetTicksSince(_start_ticks, std::chrono::duration_cast<std::chrono::milliseconds>(M2_LEVEL.GetTotalPauseDuration()).count());
}

bool m2::Timer::has_ticks_passed(thirdparty::video::Ticks ticks) const {
	return ticks < ticks_since_start();
}
