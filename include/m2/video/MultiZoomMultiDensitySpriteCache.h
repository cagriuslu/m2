#pragma once
#include <m2/video/MultiDensitySpriteCache.h>
#include <m2/common/containers/Cache.h>
#include <m2/thirdparty/video/Renderer.h>

namespace m2 {
	/// Cache of MultiDensitySpriteCaches, one per zoom level.
	class MultiZoomMultiDensitySpriteCache {
		class MultiDensitySpriteCacheGenerator {
			thirdparty::video::Renderer* _renderer;
		public:
			explicit MultiDensitySpriteCacheGenerator(thirdparty::video::Renderer& renderer) : _renderer(&renderer) {}
			MultiDensitySpriteCache operator()(int) const { return MultiDensitySpriteCache{*_renderer}; }
		};

		Cache<
				int, // Key: game height in meters, rounded to the nearest integer.
				MultiDensitySpriteCache, // Value
				MultiDensitySpriteCacheGenerator // Value generator
			> _cache;

	public:
		explicit MultiZoomMultiDensitySpriteCache(thirdparty::video::Renderer& renderer) : _cache(MultiDensitySpriteCacheGenerator{renderer}) {}

		MultiZoomMultiDensitySpriteCache(const MultiZoomMultiDensitySpriteCache&) = delete; /// Copy not allowed
		MultiZoomMultiDensitySpriteCache& operator=(const MultiZoomMultiDensitySpriteCache&) = delete; /// Copy not allowed
		MultiZoomMultiDensitySpriteCache(MultiZoomMultiDensitySpriteCache&&) noexcept = default; /// Move allowed
		MultiZoomMultiDensitySpriteCache& operator=(MultiZoomMultiDensitySpriteCache&&) noexcept = default; /// Move allowed

		/// Returns the MultiDensitySpriteCache belonging to the zoom level of `gameHeightM`, creating an empty one on
		/// first use. A zoom level is identified by the game height in meters, rounded to the nearest integer. The live
		/// game height is calculated back from the scale (see GameDimensions::SetGameHeightM), so it can differ slightly
		/// from the height that was requested. Rounding absorbs this floating point error, so the lookup never depends
		/// on an exact floating point comparison.
		MultiDensitySpriteCache& CreateOrGet(float gameHeightM);
	};
}
