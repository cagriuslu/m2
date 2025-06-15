#include <m2/video/DynamicSheet.h>
#include <m2/Log.h>

m2::DynamicSheet::DynamicSheet(SDL_Renderer* renderer, const bool darkenForLightning) : _renderer(renderer), _darkenForLightning(darkenForLightning) {
	int bpp;
	uint32_t rMask, gMask, bMask, aMask;
	SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_BGRA32, &bpp, &rMask, &gMask, &bMask, &aMask);

	sdl::SurfaceUniquePtr surface{SDL_CreateRGBSurface(0, 1024, 512, bpp, rMask, gMask, bMask, aMask)};
	m2SucceedOrThrowMessage(surface, "Unable to create surface: " + std::string{SDL_GetError()});
	_surface = std::move(surface);
}

m2::expected<m2::RectI> m2::DynamicSheet::AllocateAndMutate(const int requestedW, const int requestedH,
	const std::function<void(SDL_Surface*,const RectI&)>& mutator, bool lockSurface) {

	// Check if the requested width will fit
	if (_surface->w < requestedW) {
		return make_unexpected("Dynamic texture exceeds width limit");
	}

	RectI rect;

	// Check if the requested height would fit the current row
	if (requestedH <= _heightOfCurrentRow && requestedW <= _surface->w - _lastW) {
		rect = RectI{_lastW, _lastH, requestedW, requestedH};
		_lastW += requestedW;
	} else {
		// Resize surface if necessary
		if (_surface->h < _lastH + _heightOfCurrentRow + requestedH) {
			// Log the costly operation
			LOG_DEBUG("Doubling the height of dynamic sheet", _surface->h);
			sdl::SurfaceUniquePtr newSurface{SDL_CreateRGBSurface(0, _surface->w, _surface->h * 2,
				_surface->format->BitsPerPixel, _surface->format->Rmask, _surface->format->Gmask,
				_surface->format->Bmask, _surface->format->Amask)};
			m2ReturnUnexpectedUnless(newSurface, "Unable to create surface: " + std::string{SDL_GetError()});
			// Copy the old surface to the new
			SDL_Rect dstRect{0, 0, _surface->w, _surface->h};
			m2ReturnUnexpectedUnless(SDL_BlitSurface(_surface.get(), nullptr, newSurface.get(), &dstRect) == 0,
				"Unable to blit surface: " + std::string{SDL_GetError()});
			_surface = std::move(newSurface);
		}

		// Switch to new row
		_lastW = 0;
		_lastH += _heightOfCurrentRow;
		rect = RectI{_lastW, _lastH, requestedW, requestedH};
		_lastW += requestedW;
		_heightOfCurrentRow = requestedH;
	}

	if (lockSurface) { SDL_LockSurface(_surface.get()); }
	mutator(_surface.get(), rect);
	if (lockSurface) { SDL_UnlockSurface(_surface.get()); }

	// Recreate the texture from the surface, log the costly operation
	LOG_DEBUG("Recreating texture from the surface", _surface->h);
	sdl::TextureUniquePtr newTexture{SDL_CreateTextureFromSurface(_renderer, _surface.get())};
	m2ReturnUnexpectedUnless(newTexture, "Unable to create texture from surface: " + std::string{SDL_GetError()});
	_texture = std::move(newTexture);

	if (_darkenForLightning) {
		SDL_SetTextureColorMod(_texture.get(), 127, 127, 127);
	}

	return rect;
}
