#include <m2/thirdparty/video/Detail.h>
#include <m2/Game.h>
#include <m2/Options.h>
#include <m2/M2.h>
#include <sstream>

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

void m2::thirdparty::video::Delay(Ticks duration) {
	if (0 < duration) {
		SDL_Delay(U(duration));
	}
}

m2::thirdparty::video::Ticks m2::thirdparty::video::GetTicks() {
	return static_cast<int64_t>(SDL_GetTicks64()) / time_slowdown_factor;
}

m2::thirdparty::video::Ticks m2::thirdparty::video::GetTicksSince(Ticks lastTicks, Ticks pauseTicks) {
	return GetTicks() - lastTicks - pauseTicks;
}

int m2::thirdparty::video::GetRefreshRate() {
	SDL_DisplayMode dm{};
	SDL_GetWindowDisplayMode(M2_GAME.window, &dm);
	return dm.refresh_rate;
}
