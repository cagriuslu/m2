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
		static expected<TextTexture> CreateNoWrap(SDL_Renderer* renderer, TTF_Font* font, int fontSize,
			const std::string& text, SDL_Color color = {255, 255, 255, 255});
		/// Create a TextTexture with text wrapping. `fontSize` determines the height of the texture and `width_px`
		/// limits the number of characters per line.
		static expected<TextTexture> CreateWrapped(SDL_Renderer* renderer, TTF_Font* font, int fontSize, int width_px,
			TextHorizontalAlignment horizontal_alignment, const std::string& text,
			SDL_Color color = {255, 255, 255, 255});

		// Accessors

		explicit operator bool() const { return Texture(); }
		/// Can be null if the string is empty
		[[nodiscard]] SDL_Texture* Texture() const { return _texture.get(); }
		[[nodiscard]] VecI Dimensions() const { return Texture() ? sdl::texture_dimensions(_texture) : VecI{}; }
		[[nodiscard]] const std::string& String() const { return _string; }
	};

	/// If the text is rendered for a particular destination, the texture and the destination could be stored together
	using TextTextureAndDestination = std::pair<TextTexture,RectI>;
}
