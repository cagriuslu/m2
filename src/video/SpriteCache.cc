#include <m2/video/SpriteCache.h>

std::optional<m2::SpriteCache::Entry> m2::SpriteCache::Get(const int key) const {
	const auto it = _sprites.find(key);
	if (it == _sprites.end()) {
		return std::nullopt;
	}
	return Entry{.texture = std::cref(_dynamicTexture.Texture()), .textureRectPx = it->second.textureRectPx, .dimensionsLpx = it->second.dimensionsLpx};
}

m2::SpriteCache::Entry m2::SpriteCache::Create(const int key, const VecF& dimensionsLpx, const Painter& painter) {
	// A cached sprite is immutable; an existing one is never overwritten
	if (_sprites.contains(key)) {
		throw M2_ERROR("Sprite cache already holds a sprite under key: " + std::to_string(key));
	}
	const auto textureRectPx = m2MoveOrThrowError(_dynamicTexture.AllocateAndDraw(dimensionsLpx, painter));
	_sprites.emplace(key, CachedSprite{.textureRectPx = textureRectPx, .dimensionsLpx = dimensionsLpx});
	return Entry{.texture = std::cref(_dynamicTexture.Texture()), .textureRectPx = textureRectPx, .dimensionsLpx = dimensionsLpx};
}
