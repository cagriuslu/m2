#pragma once

#include "../VecI.h"
#include "Texture.h"
#include "../ui/Detail.h"
#include <SDL2/SDL_ttf.h>

namespace m2::sdl {
	class FontTexture {
		TextureUniquePtr _texture;
		std::string _string;

		FontTexture(SDL_Texture* texture, std::string text) : _texture(texture), _string(std::move(text)) {}

	   public:
		FontTexture() = default;
		static expected<FontTexture> create_nowrap(SDL_Renderer* renderer, TTF_Font* font, int font_size,
			const std::string& text, SDL_Color color = {255, 255, 255, 255});
		static expected<FontTexture> create_wrapped(SDL_Renderer* renderer, TTF_Font* font, int font_size,
			int font_letter_width, int width_in_chars, ui::TextHorizontalAlignment horizontal_alignment,
			const std::string& text, SDL_Color color = {255, 255, 255, 255});

		// Can be null if the string is empty
		[[nodiscard]] SDL_Texture* texture() const { return _texture.get(); }
		explicit operator bool() const { return texture(); }

		[[nodiscard]] VecI texture_dimensions() const { return texture() ? sdl::texture_dimensions(_texture) : VecI{}; }
		[[nodiscard]] const std::string& string() const { return _string; }
	};
}  // namespace m2::sdl
