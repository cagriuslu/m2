#include <m2/thirdparty/video/Detail.h>
#include "SdlConversions.h"
#include <m2/Game.h>
#include <m2/Options.h>
#include <m2/M2.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

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
	SDL_GetWindowDisplayMode(static_cast<SDL_Window*>(M2_GAME.window.RawHandle()), &dm);
	return dm.refresh_rate;
}

void m2::thirdparty::video::InitAll() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		throw M2_ERROR("SDL_Init error: " + std::string{SDL_GetError()});
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		throw M2_ERROR("IMG_Init error: " + std::string{IMG_GetError()});
	}
	if (TTF_Init() != 0) {
		throw M2_ERROR("TTF_Init error: " + std::string{TTF_GetError()});
	}

	// Default Metal backend is slow in 2.5D mode, while drawing the rectangle debug shapes
	if (SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl") == false) {
		throw M2_ERROR("Failed to set opengl as render hint");
	}
	// Use the driver line API
	if (SDL_SetHint(SDL_HINT_RENDER_LINE_METHOD, "2") == false) {
		throw M2_ERROR("Failed to set line render method");
	}
}
void m2::thirdparty::video::DeinitAll() {
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

SDL_Color  m2::thirdparty::video::ToSdlColor (const RGBA& c) { return SDL_Color{c.r, c.g, c.b, c.a}; }
SDL_Rect   m2::thirdparty::video::ToSdlRect  (const RectI& r) { return SDL_Rect{r.x, r.y, r.w, r.h}; }
SDL_Rect   m2::thirdparty::video::ToSdlRect  (const RectF& r) { return SDL_Rect{static_cast<int>(r.x), static_cast<int>(r.y), static_cast<int>(r.w), static_cast<int>(r.h)}; }
SDL_FRect  m2::thirdparty::video::ToSdlFRect (const RectF& r) { return SDL_FRect{r.x, r.y, r.w, r.h}; }
SDL_FRect  m2::thirdparty::video::ToSdlFRect (const RectI& r) { return SDL_FRect{static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h)}; }
SDL_FPoint m2::thirdparty::video::ToSdlFPoint(const VecF& v) { return SDL_FPoint{v.GetX(), v.GetY()}; }
SDL_FPoint m2::thirdparty::video::ToSdlFPoint(const VecI& v) { return SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)}; }
