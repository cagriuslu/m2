#ifndef M2_LEVELBLUEPRINT_H
#define M2_LEVELBLUEPRINT_H

#include "SpriteBlueprint.h"
#include "Def.h"

namespace m2 {
    struct TileBlueprint {
        SpriteIndex bg_sprite_index;
        SpriteIndex fg_sprite_index;
		GroupID fg_object_group;
    };

    struct LevelBlueprint {
        unsigned w, h;
        const TileBlueprint* tiles;
    };
}

#endif //M2_LEVELBLUEPRINT_H
