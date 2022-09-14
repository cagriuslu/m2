#ifndef M2_LEVELBLUEPRINT_H
#define M2_LEVELBLUEPRINT_H

#include "SpriteBlueprint.h"
#include "Group.h"

namespace m2 {
    struct TileBlueprint {
        SpriteIndex bg_sprite_index;
        SpriteIndex fg_sprite_index;
		GroupId fg_object_group;
    };

    struct LevelBlueprint {
        unsigned w, h;
        const TileBlueprint* tiles;
    };
}

#endif //M2_LEVELBLUEPRINT_H
