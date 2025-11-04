#include <m2/third_party/video/Texture.h>
#include <m2/third_party/video/Window.h>
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

void Texture::Draw2d(const RectI* patchRect, const float scale, const VecF& positionOnScreen) const {
	SDL_Rect srcRect, destRect;

	if (patchRect) {
		srcRect = static_cast<SDL_Rect>(*patchRect);
		destRect = SDL_Rect{
			I(positionOnScreen.GetX() - ToFloat(patchRect->w) * scale / 2.0f),
			I(positionOnScreen.GetY() - ToFloat(patchRect->h) * scale / 2.0f),
			CeilI(ToFloat(patchRect->w) * scale),
			CeilI(ToFloat(patchRect->h) * scale)
		};
	} else {
		int w, h;
		SDL_QueryTexture(static_cast<SDL_Texture*>(_texture), nullptr, nullptr, &w, &h);
		destRect = SDL_Rect{
			I(positionOnScreen.GetX() - ToFloat(w) * scale / 2.0f),
			I(positionOnScreen.GetY() - ToFloat(h) * scale / 2.0f),
			CeilI(ToFloat(w) * scale),
			CeilI(ToFloat(h) * scale)
		};
	}

	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	SDL_SetTextureColorMod(static_cast<SDL_Texture*>(_texture), 255, 255, 255);
	SDL_SetTextureAlphaMod(static_cast<SDL_Texture*>(_texture), 255);
	SDL_RenderCopyEx(M2_GAME.renderer, static_cast<SDL_Texture*>(_texture), patchRect ? &srcRect : nullptr, &destRect, 0.0, nullptr, SDL_FLIP_NONE);
}
