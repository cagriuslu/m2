#include <m2/thirdparty/video/Texture.h>
#include <m2/thirdparty/video/Window.h>
#include <m2/Game.h>
#include <SDL2/SDL.h>

using namespace m2;
using namespace m2::thirdparty;
using namespace m2::thirdparty::video;

Texture Texture::Generate(const int w, const int h, const std::function<RGBA(int x, int y)>& pixelGenerator) {
	const auto windowPixelFormat = GetWindowPixelFormat();
	if (SDL_BITSPERPIXEL(windowPixelFormat) != 32) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	auto* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BITSPERPIXEL(windowPixelFormat), windowPixelFormat);
	SDL_LockSurface(surface);
	auto* pixels = static_cast<uint32_t*>(surface->pixels);
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const auto rgba = pixelGenerator(x, y);
			*(pixels + x + y * w) = SDL_MapRGBA(surface->format, rgba.r, rgba.g, rgba.b, rgba.a);
		}
	}
	SDL_UnlockSurface(surface);

	auto* texture = SDL_CreateTextureFromSurface(M2_GAME.renderer, surface);
	SDL_FreeSurface(surface);
	return Texture{texture};
}
Texture Texture::CreateTargetableWindowSized() {
	const auto windowPixelFormat = GetWindowPixelFormat();
	if (SDL_BITSPERPIXEL(windowPixelFormat) != 32) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	int w, h;
	SDL_GetRendererOutputSize(M2_GAME.renderer, &w, &h); // Get screen size
	return Texture{SDL_CreateTexture(M2_GAME.renderer, windowPixelFormat, SDL_TEXTUREACCESS_TARGET, w, h)};
}
Texture Texture::CaptureWindow() {
	const auto windowPixelFormat = GetWindowPixelFormat();
	if (SDL_BITSPERPIXEL(windowPixelFormat) != 32) {
		throw M2_ERROR("Unsupported window pixel format");
	}

	int w, h;
	SDL_GetRendererOutputSize(M2_GAME.renderer, &w, &h); // Get screen size

	auto* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, SDL_BITSPERPIXEL(windowPixelFormat), windowPixelFormat);
	SDL_RenderReadPixels(M2_GAME.renderer, nullptr, windowPixelFormat, surface->pixels, surface->pitch);

	auto* texture = SDL_CreateTextureFromSurface(M2_GAME.renderer, surface);
	SDL_FreeSurface(surface);
	return Texture{texture};
}

void Texture::DrawOnto(const std::function<void()>& draw) {
	auto* const previousTarget = SDL_GetRenderTarget(M2_GAME.renderer);
	SDL_SetRenderTarget(M2_GAME.renderer, static_cast<SDL_Texture*>(_texture));
	draw();
	SDL_SetRenderTarget(M2_GAME.renderer, previousTarget);
}
void Texture::RenderToWindow() const {
	SDL_RenderCopy(M2_GAME.renderer, static_cast<SDL_Texture*>(_texture), nullptr, nullptr);
}

Texture::Texture(Texture&& other) noexcept : _texture(other._texture) {
	other._texture = nullptr;
}
Texture& Texture::operator=(Texture&& other) noexcept {
	std::swap(_texture, other._texture);
	return *this;
}
Texture::~Texture() {
	if (_texture) {
		SDL_DestroyTexture(static_cast<SDL_Texture*>(_texture));
		_texture = nullptr;
	}
}
