#include <m2/DynamicSheet.h>
#include <m2/Proxy.h>
#include <m2/Exception.h>

m2::DynamicSheet::DynamicSheet(SDL_Renderer *renderer) : _renderer(renderer) {
	int bpp;
	uint32_t r_mask, g_mask, b_mask, a_mask;
	SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_BGRA32, &bpp, &r_mask, &g_mask, &b_mask, &a_mask);

	auto* surface = SDL_CreateRGBSurface(0, 512, 512, bpp, r_mask, g_mask, b_mask, a_mask);
	if (!surface) {
		throw M2ERROR("Failed to create RGB surface: " + std::string{SDL_GetError()});
	}
	_surface.reset(surface);
}
SDL_Texture* m2::DynamicSheet::texture() const {
	return _texture.get();
}
std::pair<SDL_Surface*, SDL_Rect> m2::DynamicSheet::alloc(int w, int h) {
	// Check if effect will fit
	if (_surface->w < w) {
		throw M2FATAL("Sprite effect exceeds width limit: " + std::to_string(_surface->w));
	}

	// Resize surface if necessary
	if (_surface->h < _h + h) {
		auto* new_surface = SDL_CreateRGBSurface(0, _surface->w, (_surface->h + h) * 3 / 2, _surface->format->BitsPerPixel, _surface->format->Rmask, _surface->format->Gmask, _surface->format->Bmask, _surface->format->Amask);
		if (!new_surface) {
			throw M2ERROR("Failed to create RGB surface: " + std::string{SDL_GetError()});
		}
		SDL_Rect dstrect{0, 0, _surface->w, _surface->h};
		if (SDL_BlitSurface(_surface.get(), nullptr, new_surface, &dstrect) != 0) {
			throw M2ERROR("Failed to blit surface: " + std::string{SDL_GetError()});
		}
		_surface.reset(new_surface);
	}

	auto retval = std::make_pair(_surface.get(), SDL_Rect{0, _h, w, h});
	_h += h;
	return retval;
}
SDL_Texture* m2::DynamicSheet::recreate_texture() {
	_texture.reset(SDL_CreateTextureFromSurface(_renderer, _surface.get()));
	if (not _texture) {
		throw M2ERROR("SDL error: " + std::string{SDL_GetError()});
	}
	if (m2g::lightning) {
		// Darken the texture. TODO darken only for sprite effects
		SDL_SetTextureColorMod(_texture.get(), 127, 127, 127);
	}
	return _texture.get();
}
