#include <m2/sdl/Detail.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/M2.h>
#include <cmath>

std::string m2::ToString(const SDL_Rect& rf) {
	std::stringstream ss;
	ss << "{x:" << rf.x << ",y:" << rf.y << ",w:" << rf.w << ",h:" << rf.h << "}";
	return ss.str();
}
std::string m2::ToString(const SDL_FRect& rf) {
	std::stringstream ss;
	ss << "{x:" << rf.x << ",y:" << rf.y << ",w:" << rf.w << ",h:" << rf.h << "}";
	return ss.str();
}

void m2::sdl::delay(ticks_t duration) {
	if (0 < duration) {
		SDL_Delay(U(duration));
	}
}

m2::sdl::ticks_t m2::sdl::get_ticks() {
	return static_cast<int64_t>(SDL_GetTicks64()) / time_slowdown_factor;
}

m2::sdl::ticks_t m2::sdl::get_ticks_since(ticks_t last_ticks, ticks_t pause_ticks) {
	return get_ticks() - last_ticks - pause_ticks;
}

int m2::sdl::get_refresh_rate() {
	SDL_DisplayMode dm{};
	SDL_GetWindowDisplayMode(M2_GAME.window, &dm);
	return dm.refresh_rate;
}
