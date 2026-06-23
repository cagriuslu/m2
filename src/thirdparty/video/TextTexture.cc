#include <m2/thirdparty/video/TextTexture.h>
#include <m2/thirdparty/video/Surface.h>
#include <m2/Game.h>
#include <SDL2/SDL_ttf.h>

namespace {
	int ToTtfWrapAlignment(m2::TextHorizontalAlignment horizontalAlignment) {
		switch (horizontalAlignment) {
			case m2::TextHorizontalAlignment::LEFT:
				return TTF_WRAPPED_ALIGN_LEFT;
			case m2::TextHorizontalAlignment::RIGHT:
				return TTF_WRAPPED_ALIGN_RIGHT;
			default:
				return TTF_WRAPPED_ALIGN_CENTER;
		}
	}
}

m2::expected<m2::thirdparty::video::TextTexture> m2::thirdparty::video::TextTexture::CreateNoWrap(
		TTF_Font* font, const int fontSize, const std::string& text, const RGBA color) {
	if (text.empty()) {
		return TextTexture{std::nullopt, text};
	}
	TTF_SetFontSize(font, fontSize);
	const auto surface = Surface::RenderTextBlended(font, text, color);
	auto texture = Texture::CreateFromSurface(*M2_GAME.renderer, surface.RawHandle(), /*linearFilter=*/true);
	return TextTexture{std::make_optional(std::move(texture)), text};
}

m2::expected<m2::thirdparty::video::TextTexture> m2::thirdparty::video::TextTexture::CreateWrapped(
		TTF_Font* font, const int fontSize, const int widthPx,
		const TextHorizontalAlignment horizontalAlignment, const std::string& text, const RGBA color) {
	if (text.empty()) {
		return TextTexture{std::nullopt, text};
	}
	TTF_SetFontSize(font, fontSize);
	TTF_SetFontWrappedAlign(font, ToTtfWrapAlignment(horizontalAlignment));
	const auto surface = Surface::RenderTextBlendedWrapped(font, text, color, widthPx);
	auto texture = Texture::CreateFromSurface(*M2_GAME.renderer, surface.RawHandle(), /*linearFilter=*/true);
	return TextTexture{std::make_optional(std::move(texture)), text};
}
