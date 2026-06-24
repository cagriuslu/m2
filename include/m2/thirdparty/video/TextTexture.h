#pragma once
#include <m2/thirdparty/video/Texture.h>
#include <m2/math/VecI.h>
#include <m2/math/RectI.h>
#include <m2/video/Color.h>
#include <optional>
#include <string>
#include <utility>

#include <m2/thirdparty/video/Font.h>

namespace m2::thirdparty::video {
	class TextTexture {
		std::optional<Texture> _texture;
		std::string _string;

		TextTexture(std::optional<Texture> texture, std::string text) : _texture(std::move(texture)), _string(std::move(text)) {}

	public:
		TextTexture() = default;

		static expected<TextTexture> CreateNoWrap(Font& font, int fontSize, const std::string& text, RGBA color = RGBA::White);
		static expected<TextTexture> CreateWrapped(Font& font, int fontSize, int widthPx, TextHorizontalAlignment horizontalAlignment, const std::string& text, RGBA color = RGBA::White);

		explicit operator bool() const { return _texture.has_value(); }
		[[nodiscard]] VecI Dimensions() const { return _texture ? _texture->Dimensions() : VecI{}; }
		[[nodiscard]] const std::string& String() const { return _string; }

		void Render(const RectI& dst) const { if (_texture) _texture->Render(dst); }
		void RenderWithColorMod(const RectI& dst, const RGB& mod) const { if (_texture) _texture->RenderWithColorMod(dst, mod); }
	};

	using TextTextureAndDestination = std::pair<TextTexture, RectI>;
}
