#ifndef M2_NESTEDUI_H
#define M2_NESTEDUI_H

#include "../ElementState.h"

namespace m2::ui {
    struct UIBlueprint;
    struct UIState;
}

namespace m2::ui::element {
    struct NestedUIBlueprint {
        const UIBlueprint* ui;
    };

    struct NestedUIState : public ElementState {
        std::unique_ptr<UIState> ui;

        explicit NestedUIState(const ElementBlueprint* blueprint);
        void update_position(const SDL_Rect& rect_px) override;
        void update_content() override;
        void draw() override;
    };
}

#endif //M2_NESTEDUI_H
