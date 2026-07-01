#include <m2/thirdparty/video/TextTexture.h>
#include <m2/thirdparty/video/Surface.h>

m2::expected<m2::thirdparty::video::TextTexture> m2::thirdparty::video::TextTexture::CreateNoWrap(
		Renderer& renderer, Font& font, const int fontSize, const std::string& text, const RGBA color) {
	if (text.empty()) {
		return TextTexture{std::nullopt, text};
	}
	font.SetSize(fontSize);
	const auto surface = Surface::RenderTextBlended(font, text, color);
	auto texture = Texture::CreateFromSurface(renderer, surface.RawHandle(), /*linearFilter=*/true);
	return TextTexture{std::make_optional(std::move(texture)), text};
}

m2::expected<m2::thirdparty::video::TextTexture> m2::thirdparty::video::TextTexture::CreateWrapped(
		Renderer& renderer, Font& font, const int fontSize, const int widthSrcpx,
		const TextHorizontalAlignment horizontalAlignment, const std::string& text, const RGBA color) {
	if (text.empty()) {
		return TextTexture{std::nullopt, text};
	}
	font.SetSize(fontSize);
	font.SetWrappedAlign(horizontalAlignment);
	const auto surface = Surface::RenderTextBlendedWrapped(font, text, color, widthSrcpx);
	auto texture = Texture::CreateFromSurface(renderer, surface.RawHandle(), /*linearFilter=*/true);
	return TextTexture{std::make_optional(std::move(texture)), text};
}
