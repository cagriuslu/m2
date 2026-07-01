#include <m2/thirdparty/video/Surface.h>
#include "SdlConversions.h"
#include <m2/common/Error.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

using namespace m2;
using namespace m2::thirdparty::video;

Surface Surface::CreateFromImageFile(const std::filesystem::path& imageFilePath) {
	auto* surface = IMG_Load(imageFilePath.string().c_str());
	if (not surface) {
		throw M2_ERROR("Unable to load image: " + imageFilePath.string() + ", " + SDL_GetError());
	}
	return Surface{surface};
}
Surface Surface::CreateBlank(const int w, const int h, const uint32_t pixelFormat) {
	auto* surface = SDL_CreateSurface(w, h, static_cast<SDL_PixelFormat>(pixelFormat));
	if (not surface) {
		throw M2_ERROR("Unable to create surface: " + std::string{SDL_GetError()});
	}
	return Surface{surface};
}
Surface Surface::CreateBlankWithAlpha(const int w, const int h) {
	return CreateBlank(w, h, SDL_PIXELFORMAT_RGBA32);
}
Surface Surface::RenderTextSolid(const Font& font, const std::string& text, const RGBA& color) {
	auto* surface = TTF_RenderText_Solid(static_cast<TTF_Font*>(font.RawHandle()), text.c_str(), /*length=*/0, ToSdlColor(color));
	if (not surface) {
		throw M2_ERROR(std::string{"Unable to render text: "} + SDL_GetError());
	}
	return Surface{surface};
}
Surface Surface::RenderTextBlended(const Font& font, const std::string& text, const RGBA& color) {
	auto* surface = TTF_RenderText_Blended(static_cast<TTF_Font*>(font.RawHandle()), text.c_str(), /*length=*/0, ToSdlColor(color));
	if (not surface) {
		throw M2_ERROR(std::string{"Unable to render text: "} + SDL_GetError());
	}
	return Surface{surface};
}
Surface Surface::RenderTextBlendedWrapped(const Font& font, const std::string& text, const RGBA& color, const int wrapWidthSrcpx) {
	auto* surface = TTF_RenderText_Blended_Wrapped(static_cast<TTF_Font*>(font.RawHandle()), text.c_str(), /*length=*/0, ToSdlColor(color), wrapWidthSrcpx);
	if (not surface) {
		throw M2_ERROR(std::string{"Unable to render text: "} + SDL_GetError());
	}
	return Surface{surface};
}

VecI Surface::Dimensions() const {
	auto* const surface = static_cast<SDL_Surface*>(_surface);
	return {surface->w, surface->h};
}
uint32_t Surface::PixelFormat() const {
	return static_cast<SDL_Surface*>(_surface)->format;
}
int Surface::BytesPerPixel() const {
	return SDL_BYTESPERPIXEL(static_cast<SDL_Surface*>(_surface)->format);
}

expected<void> Surface::Blit(const Surface& source, const std::optional<RectI> sourceRect, const RectI& destinationRect) {
	SDL_Rect dst = ToSdlRect(destinationRect);
	SDL_Rect src;
	SDL_Rect* srcPtr = nullptr;
	if (sourceRect) {
		src = ToSdlRect(*sourceRect);
		srcPtr = &src;
	}
	if (not SDL_BlitSurface(static_cast<SDL_Surface*>(source._surface), srcPtr, static_cast<SDL_Surface*>(_surface), &dst)) {
		return make_unexpected("Unable to blit surface: " + std::string{SDL_GetError()});
	}
	return {};
}

void Surface::Lock() const {
	SDL_LockSurface(static_cast<SDL_Surface*>(_surface));
}
void Surface::Unlock() const {
	SDL_UnlockSurface(static_cast<SDL_Surface*>(_surface));
}

RGBA Surface::GetPixel(const int x, const int y) const {
	auto* const surface = static_cast<SDL_Surface*>(_surface);
	const auto pixel = *(static_cast<const uint32_t*>(surface->pixels) + (x + y * surface->w));
	uint8_t r, g, b, a;
	SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(surface->format), nullptr, &r, &g, &b, &a);
	return RGBA{r, g, b, a};
}
void Surface::SetPixel(const int x, const int y, const RGBA& color) {
	auto* const surface = static_cast<SDL_Surface*>(_surface);
	*(static_cast<uint32_t*>(surface->pixels) + (x + y * surface->w)) = SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), nullptr, color.r, color.g, color.b, color.a);
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
		SDL_DestroySurface(static_cast<SDL_Surface*>(_surface));
		_surface = nullptr;
	}
}
