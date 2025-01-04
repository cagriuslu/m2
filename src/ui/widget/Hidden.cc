#include <m2/ui/widget/Hidden.h>

using namespace m2;
using namespace m2::widget;

Hidden::Hidden(UiPanel* parent, const UiWidgetBlueprint* blueprint) : UiWidget(parent, blueprint) {
	// Execute on_create
	if (hidden_blueprint().on_create) {
		hidden_blueprint().on_create(*this);
	}
}

UiAction Hidden::on_event(Events& events) {
    if (const auto& on_action = hidden_blueprint().on_action; on_action) {
        if (const auto kb_shortcut = hidden_blueprint().kb_shortcut; kb_shortcut != SDL_SCANCODE_UNKNOWN) {
            if (not SDL_IsTextInputActive() && events.pop_ui_key_press(kb_shortcut)) {
                return on_action(*this);
            }
        }
    }
    return MakeContinueAction();
}

UiAction Hidden::on_update() {
    if (hidden_blueprint().on_update) {
        return hidden_blueprint().on_update(*this);
    }
    return MakeContinueAction();
}
