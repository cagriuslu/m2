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
		/// The backing surface always uses an alpha-capable pixel format, so dynamically generated textures can
		/// hold transparency regardless of the (possibly alpha-less) window pixel format.
		explicit DynamicSheet(thirdparty::video::Renderer& renderer);

		// Accessors

		[[nodiscard]] const thirdparty::video::Texture& Texture() const { return *_texture; }

		// Modifiers

		class MutableInterface {
			friend DynamicSheet;
			DynamicSheet& _sheet;
			explicit MutableInterface(DynamicSheet&);
		public:
			MutableInterface(const MutableInterface&) = delete;
			MutableInterface& operator=(const MutableInterface&) = delete;
			MutableInterface(MutableInterface&&) = delete;
			MutableInterface& operator=(MutableInterface&&) = delete;

			/// Surface must be locked only if the raw pixels will be mutated; for blitting and similar operations, the
			/// surface shouldn't be locked.
			expected<RectI> AllocateAndMutateRect(int requestedW, int requestedH, const std::function<void(thirdparty::video::Surface&,const RectI&)>& mutator, bool lockSurface = true);
		};

		/// Batches any number of allocations into a single GPU texture rebuild. The callback receives a
		/// MutableInterface it can allocate from repeatedly; the texture is recreated once, after the callback returns.
		/// Every reference previously obtained from Texture() is invalidated by this call. Rects handed out during the
		/// callback stay valid even if the surface grows, because they are absolute coordinates and growth preserves
		/// existing content at the top-left.
		void MutateSurfaceAndRecreateTexture(const std::function<void(MutableInterface&)>& cb);

	protected:
		[[nodiscard]] int Width() const { return _surface.Dimensions().x; }
		[[nodiscard]] int Height() const { return _surface.Dimensions().y; }
	};
}
