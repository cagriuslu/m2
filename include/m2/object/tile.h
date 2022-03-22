#ifndef M2_TILE_H
#define M2_TILE_H

#include <m2/object.hh>

namespace m2::object::tile {
    std::pair<Object&, ID> create(const vec2f& position, CfgSpriteIndex sprite_index);
}

#endif //M2_TILE_H
