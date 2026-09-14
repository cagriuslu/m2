#pragma once
#include <m2/common/Meta.h>
#include <m2/common/math/RectI.h>
#include <m2/common/math/VecF.h>
#include <m2/thirdparty/video/Renderer.h>
#include <m2/thirdparty/video/Texture.h>
#include <functional>

namespace m2 {
	/// Sprite sheet that dynamically increases in size on demand, backed only by a GPU texture. Unlike DynamicSheet,
	/// there is no CPU-side surface: an allocation is painted straight onto the texture with the Renderer. This has an
	/// unexpected side-effect: The renderer doesn't just copy the pixels, it renders them, blending the new pixels with
	/// the pixels at the destination. This is necessary in case the creation for a sprite requires multiple draw calls.
	/// In short, the texture will hold pixels with premultiplied alpha. Therefore, the texture must be drawn with
	/// Texture::BlendMode::PREMULTIPLIED when it's eventually drawn onto the screen.
	class DynamicTexture {
		thirdparty::video::Renderer* _renderer;
		/// Physical texture pixels per logical pixel, captured when the atlas is created. Every sprite in
		/// one atlas must use the same density.
		VecF _physicalPixelsPerLogicalPixel;
		thirdparty::video::Texture _texture;
		int _lastW{}, _lastH{}, _heightOfCurrentRow{};

	public:
		explicit DynamicTexture(thirdparty::video::Renderer& renderer);

		DynamicTexture(const DynamicTexture&) = delete; /// Copy not allowed
		DynamicTexture& operator=(const DynamicTexture&) = delete; /// Copy not allowed
		DynamicTexture(DynamicTexture&&) noexcept = default; /// Move allowed
		DynamicTexture& operator=(DynamicTexture&&) noexcept = default; /// Move allowed

		// Accessors

		/// The atlas holds premultiplied alpha, so it must be drawn with Texture::BlendMode::PREMULTIPLIED.
		[[nodiscard]] const thirdparty::video::Texture& Texture() const { return _texture; }

		// Modifiers

		/// Painter is expected to paint one allocation. It should draw in sprite-local logical pixels: (0,0) is the
		/// top-left corner of the allocation, and `dimensionsLpx` is the size that was requested. The renderer's
		/// existing logical-pixel drawing primitives apply the display density while drawing.
		using Painter = std::function<void(thirdparty::video::Renderer& renderer, const VecF& dimensionsLpx)>;
		/// Allocates `dimensionsLpx` logical pixels and invokes `painter` to fill them. Each axis is multiplied by the
		/// display density captured by the constructor and rounded up to obtain the physical texture dimensions.
		/// Returns that physical area in atlas coordinates, which is what Texture::Render takes as its source rect.
		expected<RectI> AllocateAndDraw(const VecF& dimensionsLpx, const Painter& painter);
	};
}
