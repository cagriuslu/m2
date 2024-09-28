#pragma once
#include <SDL2/SDL_ttf.h>
#include <memory>

namespace m2::sdl {
	struct FontDeleter {
		void operator()(TTF_Font* f) const { TTF_CloseFont(f); }
	};
	using FontUniquePtr = std::unique_ptr<TTF_Font, FontDeleter>;
}
