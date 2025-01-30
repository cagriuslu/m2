#include <m2/ui/widget/Hidden.h>

using namespace m2;
using namespace m2::widget;

Hidden::Hidden(UiPanel* parent, const UiWidgetBlueprint* blueprint) : UiWidget(parent, blueprint) {
	// Execute onCreate
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

UiAction Hidden::HandleEvents(Events& events) {
    if (const auto& onAction = VariantBlueprint().onAction) {
        if (const auto kb_shortcut = VariantBlueprint().kb_shortcut; kb_shortcut != SDL_SCANCODE_UNKNOWN) {
            if (not SDL_IsTextInputActive() && events.pop_ui_key_press(kb_shortcut)) {
                return onAction(*this);
            }
        }
    }
    return MakeContinueAction();
}

UiAction Hidden::UpdateContent() {
    if (VariantBlueprint().onUpdate) {
        return VariantBlueprint().onUpdate(*this);
    }
    return MakeContinueAction();
}
