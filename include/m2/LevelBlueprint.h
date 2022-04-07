#ifndef M2_LEVELBLUEPRINT_H
#define M2_LEVELBLUEPRINT_H

#include "SpriteBlueprint.h"

namespace m2 {
    struct TileBlueprint {
        SpriteIndex bg_sprite_index;
        SpriteIndex fg_sprite_index;
    };

    struct LevelBlueprint {
        unsigned w, h;
        const TileBlueprint* tiles;
    };
}

#endif //M2_LEVELBLUEPRINT_H
