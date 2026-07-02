#include <m2/video/DynamicSheet.h>
#include <m2/thirdparty/video/Window.h>
#include <m2/Log.h>

namespace {
	/// Transparent padding reserved around every glyph allocation, so that bilinear sampling of a
	/// glyph's sub-rect never bleeds into a neighboring glyph in the shared atlas.
	constexpr int GLYPH_PADDING_PX = 1;
}

m2::DynamicSheet::DynamicSheet(thirdparty::video::Renderer& renderer)
		: _renderer(renderer), _surface(thirdparty::video::Surface::CreateBlankWithAlpha(1024, 512)) {
	_texture = thirdparty::video::Texture::CreateFromSurface(_renderer, _surface.RawHandle(), /*linearFilter=*/true);
}

m2::expected<m2::RectI> m2::DynamicSheet::AllocateAndMutate(const int requestedW, const int requestedH,
	const std::function<void(thirdparty::video::Surface&,const RectI&)>& mutator, bool lockSurface) {

	const auto paddedW = requestedW + 2 * GLYPH_PADDING_PX;
	const auto paddedH = requestedH + 2 * GLYPH_PADDING_PX;

	// Check if the requested width will fit
	if (_surface.Dimensions().x < paddedW) {
		return make_unexpected("Dynamic texture exceeds width limit");
	}

	RectI allocatedRect;

	// Check if the requested height would fit the current row
	if (paddedH <= _heightOfCurrentRow && paddedW <= _surface.Dimensions().x - _lastW) {
		allocatedRect = RectI{_lastW, _lastH, paddedW, paddedH};
		_lastW += paddedW;
	} else {
		// Resize surface if necessary
		if (_surface.Dimensions().y < _lastH + _heightOfCurrentRow + paddedH) {
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
		allocatedRect = RectI{_lastW, _lastH, paddedW, paddedH};
		_lastW += paddedW;
		_heightOfCurrentRow = paddedH;
	}

	// The glyph is blitted into the centered inner rect of the padded allocation; the inner rect
	// (not the padded allocation) is what every consumer uses as the source rect.
	const RectI rect{allocatedRect.x + GLYPH_PADDING_PX, allocatedRect.y + GLYPH_PADDING_PX, requestedW, requestedH};

	if (lockSurface) { _surface.Lock(); }
	mutator(_surface, rect);
	if (lockSurface) { _surface.Unlock(); }

	// Recreate the texture from the surface, log the costly operation
	LOG_DEBUG("Recreating texture from the surface", _surface.Dimensions().y);
	_texture = thirdparty::video::Texture::CreateFromSurface(_renderer, _surface.RawHandle(), /*linearFilter=*/true);

	return rect;
}
