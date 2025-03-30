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
        if (const auto kb_shortcut = VariantBlueprint().keyboardShortcut) {
            if (not SDL_IsTextInputActive() && events.PopKeyPress(kb_shortcut)) {
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
