#include <m2/thirdparty/video/Surface.h>
#include <m2/thirdparty/video/Detail.h>
#include <m2/common/Error.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

using namespace m2;
using namespace m2::thirdparty::video;

Surface Surface::CreateFromImageFile(const std::filesystem::path& imageFilePath) {
	auto* surface = IMG_Load(imageFilePath.string().c_str());
	if (not surface) {
		throw M2_ERROR("Unable to load image: " + imageFilePath.string() + ", " + IMG_GetError());
	}
	return Surface{surface};
}
Surface Surface::CreateBlank(const int w, const int h, const uint32_t pixelFormat) {
	auto* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BITSPERPIXEL(pixelFormat), pixelFormat);
	if (not surface) {
		throw M2_ERROR("Unable to create surface: " + std::string{SDL_GetError()});
	}
	return Surface{surface};
}
Surface Surface::RenderTextSolid(const Font& font, const std::string& text, const RGBA& color) {
	auto* surface = TTF_RenderUTF8_Solid(static_cast<TTF_Font*>(font.RawHandle()), text.c_str(), ToSdlColor(color));
	if (not surface) {
		throw M2_ERROR(std::string{"Unable to render text: "} + TTF_GetError());
	}
	return Surface{surface};
}
Surface Surface::RenderTextBlended(const Font& font, const std::string& text, const RGBA& color) {
	auto* surface = TTF_RenderUTF8_Blended(static_cast<TTF_Font*>(font.RawHandle()), text.c_str(), ToSdlColor(color));
	if (not surface) {
		throw M2_ERROR(std::string{"Unable to render text: "} + TTF_GetError());
	}
	return Surface{surface};
}
Surface Surface::RenderTextBlendedWrapped(const Font& font, const std::string& text, const RGBA& color, const int wrapWidthPx) {
	auto* surface = TTF_RenderUTF8_Blended_Wrapped(static_cast<TTF_Font*>(font.RawHandle()), text.c_str(), ToSdlColor(color), wrapWidthPx);
	if (not surface) {
		throw M2_ERROR(std::string{"Unable to render text: "} + TTF_GetError());
	}
	return Surface{surface};
}

VecI Surface::Dimensions() const {
	auto* const surface = static_cast<SDL_Surface*>(_surface);
	return {surface->w, surface->h};
}

Surface::Surface(Surface&& other) noexcept : _surface(other._surface) {
	other._surface = nullptr;
}
Surface& Surface::operator=(Surface&& other) noexcept {
	std::swap(_surface, other._surface);
	return *this;
}
Surface::~Surface() {
	if (_surface) {
		SDL_FreeSurface(static_cast<SDL_Surface*>(_surface));
		_surface = nullptr;
	}
}
