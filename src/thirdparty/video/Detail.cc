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

SDL_Color  m2::thirdparty::video::ToSdlColor (const RGBA& c) { return SDL_Color{c.r, c.g, c.b, c.a}; }
SDL_Rect   m2::thirdparty::video::ToSdlRect  (const RectI& r) { return SDL_Rect{r.x, r.y, r.w, r.h}; }
SDL_Rect   m2::thirdparty::video::ToSdlRect  (const RectF& r) { return SDL_Rect{static_cast<int>(r.x), static_cast<int>(r.y), static_cast<int>(r.w), static_cast<int>(r.h)}; }
SDL_FRect  m2::thirdparty::video::ToSdlFRect (const RectF& r) { return SDL_FRect{r.x, r.y, r.w, r.h}; }
SDL_FRect  m2::thirdparty::video::ToSdlFRect (const RectI& r) { return SDL_FRect{static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h)}; }
SDL_FPoint m2::thirdparty::video::ToSdlFPoint(const VecF& v) { return SDL_FPoint{v.GetX(), v.GetY()}; }
SDL_FPoint m2::thirdparty::video::ToSdlFPoint(const VecI& v) { return SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)}; }
