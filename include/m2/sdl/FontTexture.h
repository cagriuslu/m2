#pragma once

#include <SDL2/SDL_ttf.h>
#include <m2/Meta.h>

#include "../VecI.h"
#include "Texture.h"

namespace m2::sdl {
	class FontTexture {
		TextureUniquePtr _texture;
		std::string _string;

		FontTexture(SDL_Texture* texture, std::string text) : _texture(texture), _string(std::move(text)) {}

	   public:
		FontTexture() = default;
		static expected<FontTexture> create_nowrap(TTF_Font* font, SDL_Renderer* renderer,
			const std::string& text, SDL_Color color = {255, 255, 255, 255});

		// Can be null if the string is empty
		[[nodiscard]] SDL_Texture* texture() const { return _texture.get(); }
		explicit operator bool() const { return texture(); }

		[[nodiscard]] VecI texture_dimensions() const { return texture() ? sdl::texture_dimensions(_texture) : VecI{}; }
		[[nodiscard]] const std::string& string() const { return _string; }
	};
}  // namespace m2::sdl
