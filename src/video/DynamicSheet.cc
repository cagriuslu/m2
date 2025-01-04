#include <m2/video/DynamicSheet.h>
#include <m2/Error.h>
#include <m2/Game.h>

m2::DynamicSheet::DynamicSheet(SDL_Renderer *renderer) : _renderer(renderer) {
	int bpp;
	uint32_t r_mask, g_mask, b_mask, a_mask;
	SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_BGRA32, &bpp, &r_mask, &g_mask, &b_mask, &a_mask);

	auto* surface = SDL_CreateRGBSurface(0, 16384, 512, bpp, r_mask, g_mask, b_mask, a_mask);
	if (!surface) {
		throw M2_ERROR("Failed to create RGB surface: " + std::string{SDL_GetError()});
	}
	_surface.reset(surface);
}
SDL_Texture* m2::DynamicSheet::Texture() const {
	return _texture.get();
}
std::pair<SDL_Surface*, m2::RectI> m2::DynamicSheet::Alloc(const int requestedW, const int requestedH) {
	// Check if the requested width will fit
	if (_surface->w < requestedW) {
		throw M2_ERROR("Dynamic texture exceeds width limit: " + m2::ToString(_surface->w));
	}

	// Check if the requested height would fit the current row
	if (requestedH <= _heightOfCurrentRow && requestedW <= _surface->w - _lastW) {
		auto retval = std::make_pair(_surface.get(), RectI{_lastW, _lastH, requestedW, requestedH});
		_lastW += requestedW;
		return retval;
	}

	// Resize surface if necessary
	if (_surface->h < _lastH + _heightOfCurrentRow + requestedH) {
		LOG_DEBUG("Resizing dynamic sheet of height", _surface->h);
		auto* new_surface = SDL_CreateRGBSurface(0, _surface->w, _surface->h * 2, _surface->format->BitsPerPixel,
				_surface->format->Rmask, _surface->format->Gmask, _surface->format->Bmask, _surface->format->Amask);
		if (!new_surface) {
			throw M2_ERROR("Failed to create RGB surface: " + std::string{SDL_GetError()});
		}
		SDL_Rect dstrect{0, 0, _surface->w, _surface->h};
		if (SDL_BlitSurface(_surface.get(), nullptr, new_surface, &dstrect) != 0) {
			throw M2_ERROR("Failed to blit surface: " + std::string{SDL_GetError()});
		}
		_surface.reset(new_surface);
	}

	// Switch to new row
	_lastW = 0;
	_lastH += _heightOfCurrentRow;
	auto retval = std::make_pair(_surface.get(), RectI{_lastW, _lastH, requestedW, requestedH});
	_lastW += requestedW;
	_heightOfCurrentRow = requestedH;
	return retval;
}
SDL_Texture* m2::DynamicSheet::RecreateTexture(const bool lightning) {
	_texture.reset(SDL_CreateTextureFromSurface(_renderer, _surface.get()));
	if (not _texture) {
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	}
	if (lightning) {
		// Darken the texture. TODO darken only for sprite effects
		SDL_SetTextureColorMod(_texture.get(), 127, 127, 127);
	}
	return _texture.get();
}
