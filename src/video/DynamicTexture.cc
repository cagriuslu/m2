#include <m2/video/DynamicTexture.h>
#include <m2/Log.h>

namespace {
	/// Fixed width of the atlas, in physical texture pixels. Unlike the height the width never grows, so
	/// this doubles as the hard limit on the padded width of a single allocation.
	constexpr int ATLAS_WIDTH_PX = 4096;

	/// Initial height of the atlas, in physical texture pixels. Doubles on demand, unlike the width.
	/// Deliberately far below DynamicSheet's 512: a game holding one cache per zoom level should not pay
	/// megabytes for an idle instance. 4096x64 RGBA is 1 MB.
	constexpr int INITIAL_ATLAS_HEIGHT_PX = 64;

	/// Maximum height of the atlas, in physical texture pixels. DynamicSheet grows a CPU surface, so only
	/// an over-wide allocation can fail there; a GPU texture hits GL_MAX_TEXTURE_SIZE, commonly 4096 on
	/// WebGL and mobile. 64 doubles to exactly 4096 (64 * 2^6), so growth lands on this cap without ever
	/// overshooting it, and the doubling loop never needs clamping.
	constexpr int MAX_ATLAS_HEIGHT_PX = 4096;

	/// Transparent padding reserved around every allocation, in physical texture pixels, so that filtered
	/// sampling of one sprite's sub-rect never bleeds into a neighbouring sprite.
	constexpr int SPRITE_PADDING_PX = 1;

	/// Matches DynamicSheet. Correct antialiasing given the atlas is composited premultiplied. Worth
	/// revisiting once a game renders its sprites 1:1 per zoom level, but that is a one-line change.
	constexpr bool USE_LINEAR_FILTER = true;
}

m2::DynamicTexture::DynamicTexture(thirdparty::video::Renderer& renderer)
		: _renderer(&renderer), _physicalPixelsPerLogicalPixel(renderer.GetPixelsPerWindowUnit()),
		_texture(thirdparty::video::Texture::CreateTargetableWithAlpha(renderer, ATLAS_WIDTH_PX, INITIAL_ATLAS_HEIGHT_PX, USE_LINEAR_FILTER)) {
	// SDL leaves a new target texture's contents undefined, so make them defined once, here. Allocation
	// is append-only, so this is also the only time a region needs clearing before a painter sees it.
	_texture.DrawOnto(*_renderer, [this] {
		_renderer->SetDrawColor(RGBA{0, 0, 0, 0});
		_renderer->Clear();
	});
}

m2::expected<m2::RectI> m2::DynamicTexture::AllocateAndDraw(const VecF& dimensionsLpx, const Painter& painter) {
	// Validate before touching any packing state, so a rejected request leaves the atlas untouched
	m2ReturnUnexpectedUnless(std::isfinite(dimensionsLpx.GetX()) && std::isfinite(dimensionsLpx.GetY()),
			"Sprite dimensions are not finite");
	m2ReturnUnexpectedUnless(0.0f < dimensionsLpx.GetX() && 0.0f < dimensionsLpx.GetY(),
			"Sprite dimensions are not positive");

	// One atlas must never mix sprites rasterized at different resolutions. The comparison is exact on
	// purpose: GetPixelsPerWindowUnit divides the same two integer window sizes every time, so an
	// unchanged display gives a bit-identical result, and any difference is a real density change.
	m2ReturnUnexpectedUnless(_renderer->GetPixelsPerWindowUnit() == _physicalPixelsPerLogicalPixel,
			"Display density has changed since the atlas was created");

	// Logical pixels to physical texture pixels, component-wise, so renderers whose X and Y densities
	// differ are handled too. Padding is added after the conversion, so it is one physical pixel.
	const auto scaledDimensionsPx = dimensionsLpx.Scale(_physicalPixelsPerLogicalPixel);
	const auto requestedW = CeilI(scaledDimensionsPx.GetX());
	const auto requestedH = CeilI(scaledDimensionsPx.GetY());
	const auto paddedW = requestedW + 2 * SPRITE_PADDING_PX;
	const auto paddedH = requestedH + 2 * SPRITE_PADDING_PX;
	m2ReturnUnexpectedUnless(paddedW <= ATLAS_WIDTH_PX, "Sprite exceeds the atlas width limit");

	const auto currentAtlasHeightPx = I(_texture.Dimensions().GetY());

	RectI allocatedRect;
	if (paddedH <= _heightOfCurrentRow && paddedW <= ATLAS_WIDTH_PX - _lastW) {
		allocatedRect = RectI{_lastW, _lastH, paddedW, paddedH};
		_lastW += paddedW;
	} else {
		// Grow the atlas if the new row would not fit
		auto requiredAtlasHeightPx = currentAtlasHeightPx;
		while (requiredAtlasHeightPx < _lastH + _heightOfCurrentRow + paddedH) {
			requiredAtlasHeightPx *= 2;
		}
		m2ReturnUnexpectedUnless(requiredAtlasHeightPx <= MAX_ATLAS_HEIGHT_PX,
				"Sprite exceeds the atlas height limit");
		if (currentAtlasHeightPx < requiredAtlasHeightPx) {
			// Log the costly operation
			LOG_DEBUG("Growing the height of dynamic texture", requiredAtlasHeightPx);
			auto grownTexture = thirdparty::video::Texture::CreateTargetableWithAlpha(*_renderer, ATLAS_WIDTH_PX, requiredAtlasHeightPx, USE_LINEAR_FILTER);
			grownTexture.DrawOnto(*_renderer, [this, currentAtlasHeightPx] {
				// Make the undefined contents defined, including the newly added tail below the copy
				_renderer->SetDrawColor(RGBA{0, 0, 0, 0});
				_renderer->Clear();
				// Copy the old atlas in at exactly 1:1 physical pixels. A null destination rect is
				// replaced by the current viewport, which is the only way to express an unscaled copy:
				// every Texture::Render overload scales its destination by the display density. The
				// copy must run unblended, because blending onto a transparent destination would yield
				// dstRGB = srcRGB * srcA and darken every semi-transparent pixel on every growth.
				const auto viewport = _renderer->ScopedViewport(RectI{0, 0, ATLAS_WIDTH_PX, currentAtlasHeightPx});
				const auto blendMode = _texture.ScopedBlendMode(thirdparty::video::Texture::BlendMode::NONE);
				_texture.RenderOverViewport(*_renderer);
			});
			_texture = std::move(grownTexture);
		}

		// Switch to new row
		_lastW = 0;
		_lastH += _heightOfCurrentRow;
		allocatedRect = RectI{_lastW, _lastH, paddedW, paddedH};
		_lastW += paddedW;
		_heightOfCurrentRow = paddedH;
	}

	// The sprite is painted into the centered inner rect of the padded allocation; the inner rect (not the
	// padded allocation) is what every consumer uses as the source rect, and clipping the painter to it is
	// what keeps the gutters transparent.
	const RectI rect{allocatedRect.x + SPRITE_PADDING_PX, allocatedRect.y + SPRITE_PADDING_PX, requestedW, requestedH};

	_texture.DrawOnto(*_renderer, [this, &rect, &painter, &dimensionsLpx] {
		// The viewport both translates the origin to the sprite's top-left and clips, so the painter
		// draws in sprite-local coordinates and cannot scribble on its neighbours
		const auto viewport = _renderer->ScopedViewport(rect);
		painter(*_renderer, dimensionsLpx);
	});

	return rect;
}
