#include <m2/thirdparty/video/TextTexture.h>
#include <m2/thirdparty/video/Surface.h>
#include <m2/Game.h>

m2::expected<m2::thirdparty::video::TextTexture> m2::thirdparty::video::TextTexture::CreateNoWrap(
		Font& font, const int fontSize, const std::string& text, const RGBA color) {
	if (text.empty()) {
		return TextTexture{std::nullopt, text};
	}
	font.SetSize(fontSize);
	const auto surface = Surface::RenderTextBlended(font, text, color);
	auto texture = Texture::CreateFromSurface(*M2_GAME.renderer, surface.RawHandle(), /*linearFilter=*/true);
	return TextTexture{std::make_optional(std::move(texture)), text};
}

m2::expected<m2::thirdparty::video::TextTexture> m2::thirdparty::video::TextTexture::CreateWrapped(
		Font& font, const int fontSize, const int widthPx,
		const TextHorizontalAlignment horizontalAlignment, const std::string& text, const RGBA color) {
	if (text.empty()) {
		return TextTexture{std::nullopt, text};
	}
	font.SetSize(fontSize);
	font.SetWrappedAlign(horizontalAlignment);
	const auto surface = Surface::RenderTextBlendedWrapped(font, text, color, widthPx);
	auto texture = Texture::CreateFromSurface(*M2_GAME.renderer, surface.RawHandle(), /*linearFilter=*/true);
	return TextTexture{std::make_optional(std::move(texture)), text};
}
