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

	/// Creates an empty, drawable texture the same size as the screen
	TextureUniquePtr create_drawable_texture_of_screen_size();

	/// Captures the current screen contents into a texture
	TextureUniquePtr capture_screen_as_texture();

	VecI texture_dimensions(const TextureUniquePtr& texture);
	VecI texture_dimensions(SDL_Texture* texture);

	void render_texture_with_color_mod(const TextureUniquePtr& texture, const RectI& destination, const RGB& color_mod = {255, 255, 255});
	void render_texture_with_color_mod(SDL_Texture* texture, const RectI& destination, const RGB& color_mod = {255, 255, 255});
}  // namespace m2::sdl
