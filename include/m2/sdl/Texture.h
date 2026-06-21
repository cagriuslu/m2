#pragma once
#include <SDL2/SDL.h>
#include <m2/math/VecI.h>
#include <m2/math/RectI.h>
#include <m2/video/Color.h>

namespace m2::sdl {
	struct TextureDeleter {
		void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
	};
	using TextureUniquePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;

	VecI texture_dimensions(const TextureUniquePtr& texture);
	VecI texture_dimensions(SDL_Texture* texture);
}  // namespace m2::sdl
