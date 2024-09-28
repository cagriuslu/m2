#pragma once
#include <SDL2/SDL_ttf.h>

namespace m2::sdl {
	struct FontGenerator {
		TTF_Font* operator()(int size);
	};

	struct FontCacheHashFunction {
		size_t operator()(const std::tuple<int>& cache_key) const {
			return std::hash<size_t>{}(std::get<0>(cache_key));
		}
	};
}
