#ifndef M2_SPRITEBLUEPRINT_H
#define M2_SPRITEBLUEPRINT_H

#include "Def.h"
#include "Vec2f.h"
#include <SDL.h>
#include <variant>
#include <cstdint>

namespace m2 {
    struct ColliderBlueprint {
        struct Rectangle {
            Vec2f dims_px;
            Vec2f dims_m;
        };
        struct Circle {
            float radius_px;
            float radius_m;
        };

        Vec2f center_px;
        Vec2f center_m;
        std::variant<std::monostate,Rectangle,Circle> variant;
    };

    struct SpriteBlueprint {
        SpriteIndex index{};
        SDL_Rect texture_rect{};
        Vec2f obj_center_px;
        Vec2f obj_center_m;
        ColliderBlueprint collider;
    };
}

#endif //M2_SPRITEBLUEPRINT_H
