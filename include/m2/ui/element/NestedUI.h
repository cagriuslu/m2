#ifndef M2_NESTEDUI_H
#define M2_NESTEDUI_H

#include "../ElementState.h"

namespace m2::ui {
    struct UIBlueprint;
    struct UIState;
}

namespace m2::ui::element {
    struct NestedUIBlueprint {
        const UIBlueprint* ui; // TODO convert to reference
    };

    struct NestedUIState : public ElementState {
        std::unique_ptr<UIState> ui;

        explicit NestedUIState(const ElementBlueprint* blueprint);
        void update_position(const SDL_Rect& rect_px) final;
        Action handle_events(Events& events) final;
        Action update_content() final;
        void draw() final;
    };
}

#endif //M2_NESTEDUI_H
