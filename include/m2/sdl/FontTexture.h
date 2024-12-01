#pragma once

#include "../math/VecI.h"
#include "Texture.h"
#include "../ui/Detail.h"
#include <SDL2/SDL_ttf.h>

namespace m2::sdl {
	// TODO rename to TextTexture
	class FontTexture {
		TextureUniquePtr _texture;
		std::string _string;

		FontTexture(SDL_Texture* texture, std::string text) : _texture(texture), _string(std::move(text)) {}

	public:
		FontTexture() = default;
		/// Create a FontTexture without text wrapping. `font_size` determines the height of the texture, and since the
		/// text is unwrapped, the width is also determined.
		static expected<FontTexture> create_nowrap(SDL_Renderer* renderer, TTF_Font* font, int font_size,
			const std::string& text, SDL_Color color = {255, 255, 255, 255});
		/// Create a FontTexture with text wrapping. `font_size` determines the height of the texture and `width_px`
		/// limits the number of characters per line.
		static expected<FontTexture> create_wrapped(SDL_Renderer* renderer, TTF_Font* font, int font_size, int width_px,
			ui::TextHorizontalAlignment horizontal_alignment, const std::string& text,
			SDL_Color color = {255, 255, 255, 255});

		// Can be null if the string is empty
		[[nodiscard]] SDL_Texture* texture() const { return _texture.get(); }
		explicit operator bool() const { return texture(); }

		[[nodiscard]] VecI texture_dimensions() const { return texture() ? sdl::texture_dimensions(_texture) : VecI{}; }
		[[nodiscard]] const std::string& string() const { return _string; }
	};

	/// FontTexture and its destination is sometimes grouped together if the text is created for that destination.
	struct FontTextureAndDestination {
		FontTexture font_texture;
		RectI destination_rect;
	};
}  // namespace m2::sdl
