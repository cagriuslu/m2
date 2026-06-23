#include <m2/video/DynamicSheet.h>
#include <m2/Log.h>
#include <SDL2/SDL_pixels.h> // For SDL_PIXELFORMAT_BGRA32; pixel-format constants are not yet abstracted

m2::DynamicSheet::DynamicSheet(thirdparty::video::Renderer& renderer)
		: _renderer(renderer), _surface(thirdparty::video::Surface::CreateBlank(1024, 512, SDL_PIXELFORMAT_BGRA32)) {
	_texture = thirdparty::video::Texture::CreateFromSurface(_renderer, _surface.RawHandle());
}

m2::expected<m2::RectI> m2::DynamicSheet::AllocateAndMutate(const int requestedW, const int requestedH,
	const std::function<void(thirdparty::video::Surface&,const RectI&)>& mutator, bool lockSurface) {

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
			LOG_DEBUG("Doubling the height of dynamic sheet", _surface.Dimensions().y);
			auto newSurface = thirdparty::video::Surface::CreateBlank(_surface.Dimensions().x, _surface.Dimensions().y * 2, _surface.PixelFormat());
			auto blitResult = newSurface.Blit(_surface, std::nullopt, RectI{0, 0, _surface.Dimensions().x, _surface.Dimensions().y});
			m2ReflectUnexpected(blitResult);
			_surface = std::move(newSurface);
		}

		// Switch to new row
		_lastW = 0;
		_lastH += _heightOfCurrentRow;
		rect = RectI{_lastW, _lastH, requestedW, requestedH};
		_lastW += requestedW;
		_heightOfCurrentRow = requestedH;
	}

	if (lockSurface) { _surface.Lock(); }
	mutator(_surface, rect);
	if (lockSurface) { _surface.Unlock(); }

	// Recreate the texture from the surface, log the costly operation
	LOG_DEBUG("Recreating texture from the surface", _surface.Dimensions().y);
	_texture = thirdparty::video::Texture::CreateFromSurface(_renderer, _surface.RawHandle());

	return rect;
}
