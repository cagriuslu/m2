#ifndef M2_TILE_H
#define M2_TILE_H

#include <m2/Object.h>
#include <m2/SpriteBlueprint.h>
#include <m2/Sprite.h>

namespace m2::obj {
	std::pair<Object&, Id> create_tile(const Vec2f& position, const m2::Sprite& sprite);
}

#endif //M2_TILE_H
