#pragma once
#include <m2/common/Meta.h>
#include "../math/VecI.h"
#include <RectI.pb.h>
#include "../math/RectI.h"
#include <SDL2/SDL.h>

namespace m2 {
	std::string ToString(const SDL_Rect& rf);
	std::string ToString(const SDL_FRect& rf);
}

namespace m2::sdl {
	using ticks_t = int64_t;
	void delay(ticks_t duration);
	ticks_t get_ticks();
	ticks_t get_ticks_since(ticks_t last_ticks, ticks_t pause_ticks = 0);

	int get_refresh_rate();
}
