#ifndef M2_TILE_H
#define M2_TILE_H

#include <m2/Object.h>

namespace m2::object {
    std::pair<Object&, ID> create_tile(const vec2f& position, CfgSpriteIndex sprite_index);
}

#endif //M2_TILE_H
