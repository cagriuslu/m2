#pragma once
#include "../UiAction.h"
#include <SDL.h>

namespace m2::widget {
    // Forward declaration
    class Hidden;

    // Empty widget with no visuals
    struct HiddenBlueprint {
        m2g::pb::KeyType keyboardShortcut{};

		std::function<void(const Hidden& self)> onCreate{};
        std::function<UiAction(const Hidden& self)> onUpdate{};
        std::function<UiAction(const Hidden& self)> onAction{};
    };
}
