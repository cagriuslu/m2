#pragma once
#include "math/RectI.h"

namespace m2 {
    struct GameDimensions {
        int height_m{20};  // Controls the zoom of the game
        float width_m{};
        int ppm{};
        RectI window{}, game{}, game_and_hud{};
        RectI top_envelope{}, bottom_envelope{}, left_envelope{}, right_envelope{};
        RectI left_hud{}, right_hud{}, message_box{};

        GameDimensions() = default;
        GameDimensions(int game_height_m, int window_width, int window_height, int game_aspect_ratio_mul, int game_aspect_ratio_div);

        float hud_width_to_game_and_hud_width_ratio() const;
        float game_width_to_game_and_hud_width_ration() const;
    };
}
