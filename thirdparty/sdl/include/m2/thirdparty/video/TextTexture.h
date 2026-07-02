#pragma once
#include <m2/thirdparty/video/Texture.h>
#include <m2/thirdparty/video/Font.h>
#include <m2/common/math/VecI.h>
#include <m2/common/math/RectF.h>
#include <m2/common/math/RectI.h>
#include <m2/common/video/Color.h>
#include <optional>
#include <string>
#include <utility>

namespace m2::thirdparty::video {
	class TextTexture {
		std::optional<Texture> _texture;
		std::string _string;
		float _dpiScale = 1.0f;

		TextTexture(std::optional<Texture> texture, std::string text, float dpiScale = 1.0f) :
			_texture(std::move(texture)), _string(std::move(text)), _dpiScale(dpiScale) {}

	public:
		TextTexture() = default;

		static expected<TextTexture> CreateNoWrap(Renderer& renderer, Font& font, int fontSize, const std::string& text, RGBA color = RGBA::White);
		static expected<TextTexture> CreateWrapped(Renderer& renderer, Font& font, int fontSize, int widthSrcpx, TextHorizontalAlignment horizontalAlignment, const std::string& text, RGBA color = RGBA::White);

		explicit operator bool() const { return _texture.has_value(); }
		/// In logical pixels
		[[nodiscard]] VecF Dimensions() const { return _texture ? _texture->Dimensions() / _dpiScale : VecF{}; }
		[[nodiscard]] const std::string& String() const { return _string; }

		void Render(Renderer& renderer, const RectF& dst) const { if (_texture) _texture->Render(renderer, dst); }
		void RenderWithColorMod(Renderer& renderer, const RectF& dst, const RGB& mod) const { if (_texture) _texture->RenderWithColorMod(renderer, dst, mod); }
	};

	using TextTextureAndDestination = std::pair<TextTexture, RectF>;
}
