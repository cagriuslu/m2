#pragma once

#include "../math/VecI.h"
#include "Texture.h"
#include "../ui/Detail.h"
#include <SDL2/SDL_ttf.h>

namespace m2::sdl {
	class TextTexture {
		TextureUniquePtr _texture;
		std::string _string;

		TextTexture(SDL_Texture* texture, std::string text) : _texture(texture), _string(std::move(text)) {}

	public:
		TextTexture() = default;
		/// Create a TextTexture without text wrapping. `fontSize` determines the height of the texture, and since the
		/// text is unwrapped, the width is also determined.
		static expected<TextTexture> create_nowrap(SDL_Renderer* renderer, TTF_Font* font, int fontSize,
			const std::string& text, SDL_Color color = {255, 255, 255, 255});
		/// Create a TextTexture with text wrapping. `fontSize` determines the height of the texture and `width_px`
		/// limits the number of characters per line.
		static expected<TextTexture> create_wrapped(SDL_Renderer* renderer, TTF_Font* font, int fontSize, int width_px,
			TextHorizontalAlignment horizontal_alignment, const std::string& text,
			SDL_Color color = {255, 255, 255, 255});

		// Can be null if the string is empty
		[[nodiscard]] SDL_Texture* texture() const { return _texture.get(); }
		explicit operator bool() const { return texture(); }

		[[nodiscard]] VecI texture_dimensions() const { return texture() ? sdl::texture_dimensions(_texture) : VecI{}; }
		[[nodiscard]] const std::string& string() const { return _string; }
	};

	/// TextTexture and its destination is sometimes grouped together if the text is created for that destination.
	struct TextTextureAndDestination {
		TextTexture textTexture;
		RectI destinationRect;
	};
}  // namespace m2::sdl
