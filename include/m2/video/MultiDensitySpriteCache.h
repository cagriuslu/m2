#pragma once
#include <m2/video/SpriteCache.h>
#include <m2/common/containers/Cache.h>
#include <m2/common/math/VecF.h>
#include <m2/thirdparty/video/Renderer.h>

namespace m2 {
	/// Cache of SpriteCaches, one per window pixel density (Renderer::GetPixelsPerWindowUnit()). A SpriteCache
	/// rasterizes its sprites for exactly one density and refuses to grow once the display density changes, so a
	/// game that survives a move to a differently-scaled display needs one SpriteCache per density it has seen.
	class MultiDensitySpriteCache {
		class SpriteCacheGenerator {
			thirdparty::video::Renderer* _renderer;
		public:
			explicit SpriteCacheGenerator(thirdparty::video::Renderer& renderer) : _renderer(&renderer) {}
			SpriteCache operator()(const VecF&) const { return SpriteCache{*_renderer}; }
		};
		struct DensityHash {
			size_t operator()(const VecF& density) const;
		};

		Cache<
				VecF, // Key
				SpriteCache, // Value
				SpriteCacheGenerator, // Value generator
				DensityHash // Key hash function
			> _cache;

	public:
		explicit MultiDensitySpriteCache(thirdparty::video::Renderer& renderer) : _cache(SpriteCacheGenerator{renderer}) {}

		MultiDensitySpriteCache(const MultiDensitySpriteCache&) = delete; /// Copy not allowed
		MultiDensitySpriteCache& operator=(const MultiDensitySpriteCache&) = delete; /// Copy not allowed
		MultiDensitySpriteCache(MultiDensitySpriteCache&&) noexcept = default; /// Move allowed
		MultiDensitySpriteCache& operator=(MultiDensitySpriteCache&&) noexcept = default; /// Move allowed

		/// Returns the SpriteCache belonging to `density`, creating an empty one on first use. Density is expected from
		/// the user instead of being inferred from the renderer to allow the user to cache the density.
		SpriteCache& CreateOrGet(const VecF& density);
	};
}
