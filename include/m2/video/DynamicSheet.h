#pragma once
#include <m2/common/math/RectI.h>
#include <m2/thirdparty/video/Renderer.h>
#include <m2/thirdparty/video/Surface.h>
#include <m2/thirdparty/video/Texture.h>
#include <optional>

namespace m2 {
	/// Sprite sheet that dynamically increases in size on demand
	class DynamicSheet {
		thirdparty::video::Renderer& _renderer;
		thirdparty::video::Surface _surface;
		std::optional<thirdparty::video::Texture> _texture;
		int _lastW{}, _lastH{}, _heightOfCurrentRow{};

	public:
		/// If pixelFormat is zero, the window pixel format is looked up
		explicit DynamicSheet(thirdparty::video::Renderer& renderer, uint32_t pixelFormat);

		// Accessors

		[[nodiscard]] const thirdparty::video::Texture& Texture() const { return *_texture; }

		// Modifiers

		/// Tries to allocate the requested area on the dynamic surface, optionally locks the surface, and calls the
		/// mutator with the surface and the area. The same area is returned at the end. Surface must be locked only if
		/// the raw pixels will be mutated. For blitting and similar operations, the surface shouldn't be locked.
		expected<RectI> AllocateAndMutate(int requestedW, int requestedH, const std::function<void(thirdparty::video::Surface&,const RectI&)>& mutator, bool lockSurface = true);

	protected:
		[[nodiscard]] int Width() const { return _surface.Dimensions().x; }
		[[nodiscard]] int Height() const { return _surface.Dimensions().y; }
	};
}
