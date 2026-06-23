#pragma once
#include <SDL2/SDL.h>
#include <m2/video/Color.h>
#include <m2/math/RectI.h>
#include <m2/math/RectF.h>
#include <m2/math/VecF.h>
#include <m2/math/VecI.h>
#include <cstdint>
#include <string>

namespace m2 {
	std::string ToString(const SDL_Rect& rf);
	std::string ToString(const SDL_FRect& rf);
}

namespace m2::thirdparty::video {
	using Ticks = int64_t;
	void Delay(Ticks duration);
	Ticks GetTicks();
	Ticks GetTicksSince(Ticks lastTicks, Ticks pauseTicks = 0);

	int GetRefreshRate();

	void InitAll();
	void DeinitAll();

	// TODO remove these, SDL types shouldn't leak into the game/engine
	SDL_Color  ToSdlColor (const RGBA& c);
	SDL_Rect   ToSdlRect  (const RectI& r);
	SDL_Rect   ToSdlRect  (const RectF& r);
	SDL_FRect  ToSdlFRect (const RectF& r);
	SDL_FRect  ToSdlFRect (const RectI& r);
	SDL_FPoint ToSdlFPoint(const VecF& v);
	SDL_FPoint ToSdlFPoint(const VecI& v);
}
