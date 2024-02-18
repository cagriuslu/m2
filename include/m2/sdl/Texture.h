#pragma once
#include <SDL2/SDL.h>
#include <m2/VecI.h>

namespace m2::sdl {
	struct TextureDeleter {
		void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
	};
	using TextureUniquePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;

	TextureUniquePtr capture_screen_as_texture();
	VecI texture_dimensions(const TextureUniquePtr& texture);
	VecI texture_dimensions(SDL_Texture* texture);
}  // namespace m2::sdl
