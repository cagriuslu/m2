#pragma once
#include <SDL2/SDL.h>
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
}
