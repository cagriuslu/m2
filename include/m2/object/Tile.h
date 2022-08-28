#ifndef M2_TILE_H
#define M2_TILE_H

#include <m2/Object.h>
#include <m2/SpriteBlueprint.h>
#include <m2/Sprite.h>

namespace m2::obj {
    std::pair<Object&, ID> create_tile(const Vec2f& position, m2::SpriteIndex sprite_index);
}

#endif //M2_TILE_H
