#ifndef M2_NESTEDUI_H
#define M2_NESTEDUI_H

#include "../WidgetState.h"

namespace m2::ui {
    struct UIBlueprint;
    struct UIState;
}

namespace m2::ui::wdg {
    struct NestedUIBlueprint {
        const UIBlueprint* ui; // TODO convert to reference
    };

    struct NestedUIState : public WidgetState {
        std::unique_ptr<UIState> ui;

        explicit NestedUIState(const WidgetBlueprint* blueprint);
        void update_position(const SDL_Rect& rect_px) final;
        Action handle_events(Events& events) final;
        Action update_content() final;
        void draw() final;
    };
}

#endif //M2_NESTEDUI_H
