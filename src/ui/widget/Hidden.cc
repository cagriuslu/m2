#include <m2/ui/widget/Hidden.h>

using namespace m2::ui;
using namespace m2::ui::widget;

Hidden::Hidden(State* parent, const WidgetBlueprint* blueprint) : Widget(parent, blueprint) {
	// Execute on_create
	if (hidden_blueprint().on_create) {
		hidden_blueprint().on_create(*this);
	}
}

Action Hidden::on_event(Events& events) {
    if (const auto& on_action = hidden_blueprint().on_action; on_action) {
        if (const auto kb_shortcut = hidden_blueprint().kb_shortcut; kb_shortcut != SDL_SCANCODE_UNKNOWN) {
            if (not SDL_IsTextInputActive() && events.pop_ui_key_press(kb_shortcut)) {
                return on_action(*this);
            }
        }
    }
    return make_continue_action();
}

Action Hidden::on_update() {
    if (hidden_blueprint().on_update) {
        return hidden_blueprint().on_update(*this);
    }
    return make_continue_action();
}
