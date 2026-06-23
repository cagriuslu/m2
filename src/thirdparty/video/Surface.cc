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
uint32_t Surface::PixelFormat() const {
	return static_cast<SDL_Surface*>(_surface)->format->format;
}
int Surface::BytesPerPixel() const {
	return static_cast<SDL_Surface*>(_surface)->format->BytesPerPixel;
}

expected<void> Surface::Blit(const Surface& source, const std::optional<RectI> sourceRect, const RectI& destinationRect) {
	SDL_Rect dst = ToSdlRect(destinationRect);
	SDL_Rect src;
	SDL_Rect* srcPtr = nullptr;
	if (sourceRect) {
		src = ToSdlRect(*sourceRect);
		srcPtr = &src;
	}
	if (SDL_BlitSurface(static_cast<SDL_Surface*>(source._surface), srcPtr, static_cast<SDL_Surface*>(_surface), &dst) != 0) {
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
	SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
	return RGBA{r, g, b, a};
}
void Surface::SetPixel(const int x, const int y, const RGBA& color) {
	auto* const surface = static_cast<SDL_Surface*>(_surface);
	*(static_cast<uint32_t*>(surface->pixels) + (x + y * surface->w)) = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
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
