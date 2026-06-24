#pragma once
#include <m2/common/video/Color.h>
#include <m2/common/math/RectI.h>
#include <m2/common/math/RectF.h>
#include <m2/common/math/VecF.h>
#include <m2/common/math/VecI.h>
#include <SDL2/SDL.h>

namespace m2::thirdparty::video {
	SDL_Color  ToSdlColor (const RGBA& c);
	SDL_Rect   ToSdlRect  (const RectI& r);
	SDL_Rect   ToSdlRect  (const RectF& r);
	SDL_FRect  ToSdlFRect (const RectF& r);
	SDL_FRect  ToSdlFRect (const RectI& r);
	SDL_FPoint ToSdlFPoint(const VecF& v);
	SDL_FPoint ToSdlFPoint(const VecI& v);
}
