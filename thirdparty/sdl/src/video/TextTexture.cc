#include <m2/thirdparty/video/TextTexture.h>
#include <m2/thirdparty/video/Surface.h>
#include <m2/thirdparty/video/Renderer.h>
#include <m2/common/Meta.h>

m2::expected<m2::thirdparty::video::TextTexture> m2::thirdparty::video::TextTexture::CreateNoWrap(
		Renderer& renderer, Font& font, const int fontSize, const std::string& text, const RGBA color) {
	if (text.empty()) {
		return TextTexture{std::nullopt, text};
	}
	const float dpiX = renderer.GetPixelsPerWindowUnit().GetX();
	font.SetSize(ToFloat(fontSize) * dpiX);
	const auto surface = Surface::RenderTextBlended(font, text, color);
	auto texture = Texture::CreateFromSurface(renderer, surface.RawHandle(), /*linearFilter=*/true);
	return TextTexture{std::make_optional(std::move(texture)), text, dpiX};
}

m2::expected<m2::thirdparty::video::TextTexture> m2::thirdparty::video::TextTexture::CreateWrapped(
		Renderer& renderer, Font& font, const int fontSize, const int widthSrcpx,
		const TextHorizontalAlignment horizontalAlignment, const std::string& text, const RGBA color) {
	if (text.empty()) {
		return TextTexture{std::nullopt, text};
	}
	const float dpiX = renderer.GetPixelsPerWindowUnit().GetX();
	font.SetSize(ToFloat(fontSize) * dpiX);
	font.SetWrappedAlign(horizontalAlignment);
	const auto surface = Surface::RenderTextBlendedWrapped(font, text, color, I(ToFloat(widthSrcpx) * dpiX));
	auto texture = Texture::CreateFromSurface(renderer, surface.RawHandle(), /*linearFilter=*/true);
	return TextTexture{std::make_optional(std::move(texture)), text, dpiX};
}
