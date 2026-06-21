#pragma once

#include "../math/VecI.h"
#include <m2/thirdparty/video/Texture.h>
#include "../ui/Detail.h"
#include <SDL2/SDL_ttf.h>
#include <optional>

namespace m2::sdl {
	class TextTexture {
		std::optional<thirdparty::video::Texture> _texture;
		std::string _string;

		TextTexture(SDL_Texture* rawTexture, std::string text)
			: _texture(rawTexture ? std::make_optional(thirdparty::video::Texture::AdoptRawTexture(rawTexture)) : std::nullopt),
			  _string(std::move(text)) {}

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

		explicit operator bool() const { return _texture.has_value(); }
		[[nodiscard]] VecI Dimensions() const { return _texture ? _texture->Dimensions() : VecI{}; }
		[[nodiscard]] const std::string& String() const { return _string; }

		void Render(const RectI& dst) const { if (_texture) _texture->Render(dst); }
		void RenderWithColorMod(const RectI& dst, const RGB& mod) const { if (_texture) _texture->RenderWithColorMod(dst, mod); }
	};

	/// If the text is rendered for a particular destination, the texture and the destination could be stored together
	using TextTextureAndDestination = std::pair<TextTexture,RectI>;
}
