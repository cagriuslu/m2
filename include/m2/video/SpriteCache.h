#pragma once
#include <m2/video/DynamicTexture.h>
#include <m2/common/math/RectI.h>
#include <m2/common/math/VecF.h>
#include <functional>
#include <optional>
#include <unordered_map>

namespace m2 {
	/// Cache of game-drawn sprites, packed into one GPU texture. A cached sprite is immutable. To re-rasterize at a
	/// different resolution, destroy the cache and build a new one. The lookup key is an int.
	class SpriteCache {
		DynamicTexture _dynamicTexture;
		struct CachedSprite {
			RectI textureRectPx;
			VecF dimensionsLpx;
		};
		std::unordered_map<int, CachedSprite> _sprites;

	public:
		explicit SpriteCache(thirdparty::video::Renderer& renderer) : _dynamicTexture(renderer) {}

		// Accessors

		struct Entry {
			/// The atlas holds premultiplied alpha. Utilize a ScopedBlendMode(Texture::BlendMode::PREMULTIPLIED) guard
			/// before the Render call.
			std::reference_wrapper<const thirdparty::video::Texture> texture;
			RectI textureRectPx;
			VecF dimensionsLpx;
		};
		/// Returns the sprite stored under `key`, or nullopt if there is none.
		[[nodiscard]] std::optional<Entry> Get(int key) const;

		/// The atlas holds premultiplied alpha, so it must be drawn with Texture::BlendMode::PREMULTIPLIED.
		[[nodiscard]] const thirdparty::video::Texture& Texture() const { return _dynamicTexture.Texture(); }

		// Modifiers

		/// See DynamicTexture::Painter.
		using Painter = DynamicTexture::Painter;
		/// Paints a sprite of `dimensionsLpx` logical pixels and stores it under `key`. Throws if `key`
		/// already holds a sprite, because an existing sprite is never overwritten, and throws if the
		/// atlas cannot fit the request or its captured display density is stale.
		Entry Create(int key, const VecF& dimensionsLpx, const Painter& painter);
	};
}
