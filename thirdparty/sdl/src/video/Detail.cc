#include <m2/thirdparty/video/Detail.h>
#include "SdlConversions.h"
#include <m2/common/Error.h>
#include <m2/common/Meta.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

void m2::thirdparty::video::Delay(Ticks duration) {
	if (0 < duration) {
		SDL_Delay(U(duration));
	}
}

m2::thirdparty::video::Ticks m2::thirdparty::video::GetTicks() {
	return static_cast<int64_t>(SDL_GetTicks());
}

m2::thirdparty::video::Ticks m2::thirdparty::video::GetTicksSince(Ticks lastTicks, Ticks pauseTicks) {
	return GetTicks() - lastTicks - pauseTicks;
}

void m2::thirdparty::video::InitAll() {
	if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
		throw M2_ERROR("SDL_Init error: " + std::string{SDL_GetError()});
	}
	if (not TTF_Init()) {
		throw M2_ERROR("TTF_Init error: " + std::string{SDL_GetError()});
	}
}
void m2::thirdparty::video::DeinitAll() {
	TTF_Quit();
	SDL_Quit();
}

SDL_Color  m2::thirdparty::video::ToSdlColor (const RGBA& c) { return SDL_Color{c.r, c.g, c.b, c.a}; }
SDL_FColor m2::thirdparty::video::ToSdlFColor(const RGBA& c) { return SDL_FColor{c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f}; }
SDL_Rect   m2::thirdparty::video::ToSdlRect  (const RectI& r) { return SDL_Rect{r.x, r.y, r.w, r.h}; }
SDL_Rect   m2::thirdparty::video::ToSdlRect  (const RectF& r) { return SDL_Rect{static_cast<int>(r.x), static_cast<int>(r.y), static_cast<int>(r.w), static_cast<int>(r.h)}; }
SDL_FRect  m2::thirdparty::video::ToSdlFRect (const RectF& r) { return SDL_FRect{r.x, r.y, r.w, r.h}; }
SDL_FRect  m2::thirdparty::video::ToSdlFRect (const RectI& r) { return SDL_FRect{static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h)}; }
SDL_FPoint m2::thirdparty::video::ToSdlFPoint(const VecF& v) { return SDL_FPoint{v.GetX(), v.GetY()}; }
SDL_FPoint m2::thirdparty::video::ToSdlFPoint(const VecI& v) { return SDL_FPoint{static_cast<float>(v.x), static_cast<float>(v.y)}; }
