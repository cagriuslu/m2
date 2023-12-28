#pragma once
#include "../Action.h"
#include <SDL.h>

namespace m2::ui::widget {
    // Forward declaration
    class Hidden;

    // Empty widget with no visuals
    struct HiddenBlueprint {
        SDL_Scancode kb_shortcut{};

		std::function<void(const Hidden& self)> on_create{};
        std::function<Action(const Hidden& self)> on_action{};
        std::function<Action(const Hidden& self)> on_update{};
    };
}
