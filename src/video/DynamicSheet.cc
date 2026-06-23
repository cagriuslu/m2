#include <m2/video/DynamicSheet.h>
#include <m2/Log.h>

m2::DynamicSheet::DynamicSheet(thirdparty::video::Renderer& renderer)
		: _renderer(renderer), _surface(thirdparty::video::Surface::CreateBlank(1024, 512, SDL_PIXELFORMAT_BGRA32)) {
	_texture = thirdparty::video::Texture::CreateFromSurface(_renderer, _surface.RawHandle());
}

m2::expected<m2::RectI> m2::DynamicSheet::AllocateAndMutate(const int requestedW, const int requestedH,
	const std::function<void(SDL_Surface*,const RectI&)>& mutator, bool lockSurface) {

	// Check if the requested width will fit
	if (_surface.Dimensions().x < requestedW) {
		return make_unexpected("Dynamic texture exceeds width limit");
	}

	RectI rect;

	// Check if the requested height would fit the current row
	if (requestedH <= _heightOfCurrentRow && requestedW <= _surface.Dimensions().x - _lastW) {
		rect = RectI{_lastW, _lastH, requestedW, requestedH};
		_lastW += requestedW;
	} else {
		// Resize surface if necessary
		if (_surface.Dimensions().y < _lastH + _heightOfCurrentRow + requestedH) {
			// Log the costly operation
			auto* rawSurface = static_cast<SDL_Surface*>(_surface.RawHandle());
			LOG_DEBUG("Doubling the height of dynamic sheet", rawSurface->h);
			auto newSurface = thirdparty::video::Surface::CreateBlank(rawSurface->w, rawSurface->h * 2, rawSurface->format->format);
			SDL_Rect dstRect{0, 0, rawSurface->w, rawSurface->h};
			m2ReturnUnexpectedUnless(
				SDL_BlitSurface(rawSurface, nullptr, static_cast<SDL_Surface*>(newSurface.RawHandle()), &dstRect) == 0,
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

	auto* lockableSurface = static_cast<SDL_Surface*>(_surface.RawHandle());
	if (lockSurface) { SDL_LockSurface(lockableSurface); }
	mutator(lockableSurface, rect);
	if (lockSurface) { SDL_UnlockSurface(lockableSurface); }

	// Recreate the texture from the surface, log the costly operation
	LOG_DEBUG("Recreating texture from the surface", _surface.Dimensions().y);
	_texture = thirdparty::video::Texture::CreateFromSurface(_renderer, _surface.RawHandle());

	return rect;
}
