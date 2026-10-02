#pragma once
#include <m2/video/SpriteCache.h>
#include <m2/common/containers/Cache.h>
#include <m2/thirdparty/video/Renderer.h>

namespace m2 {
	/// Cache of SpriteCaches, one per zoom level. Each SpriteCache rasterizes its sprites for the display density that
	/// is live when the SpriteCache is created. To re-rasterize after a display density change, destroy this cache and
	/// build a new one.
	class MultiZoomSpriteCache {
		class SpriteCacheGenerator {
			thirdparty::video::Renderer* _renderer;
		public:
			explicit SpriteCacheGenerator(thirdparty::video::Renderer& renderer) : _renderer(&renderer) {}
			SpriteCache operator()(int) const { return SpriteCache{*_renderer}; }
		};

		Cache<
				int, // Key: game height in meters, rounded to the nearest integer.
				SpriteCache, // Value
				SpriteCacheGenerator // Value generator
			> _cache;

	public:
		explicit MultiZoomSpriteCache(thirdparty::video::Renderer& renderer) : _cache(SpriteCacheGenerator{renderer}) {}

		MultiZoomSpriteCache(const MultiZoomSpriteCache&) = delete; /// Copy not allowed
		MultiZoomSpriteCache& operator=(const MultiZoomSpriteCache&) = delete; /// Copy not allowed
		MultiZoomSpriteCache(MultiZoomSpriteCache&&) noexcept = default; /// Move allowed
		MultiZoomSpriteCache& operator=(MultiZoomSpriteCache&&) noexcept = default; /// Move allowed

		/// Returns the SpriteCache belonging to the zoom level of `gameHeightM`, creating an empty one on
		/// first use. A zoom level is identified by the game height in meters, rounded to the nearest integer. The live
		/// game height is calculated back from the scale (see GameDimensions::SetGameHeightM), so it can differ slightly
		/// from the height that was requested. Rounding absorbs this floating point error, so the lookup never depends
		/// on an exact floating point comparison.
		SpriteCache& CreateOrGet(float gameHeightM);
	};
}
