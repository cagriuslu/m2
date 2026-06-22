#include <m2/ui/widget/Hidden.h>
#include <m2/thirdparty/event/Event.h>

using namespace m2;
using namespace m2::widget;

Hidden::Hidden(UiPanel* parent, const UiWidgetBlueprint* blueprint) : UiWidget(parent, blueprint) {
	// Execute onCreate
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

UiAction Hidden::OnEvent(Events& events) {
    if (const auto& onAction = VariantBlueprint().onAction) {
        if (const auto kb_shortcut = VariantBlueprint().keyboardShortcut) {
            if (not m2::thirdparty::event::IsTextInputActive() && events.PopKeyPress(kb_shortcut)) {
                return onAction(*this);
            }
        }
    }
    return MakeContinueAction();
}

UiAction Hidden::OnUpdate() {
    if (VariantBlueprint().onUpdate) {
        return VariantBlueprint().onUpdate(*this);
    }
    return MakeContinueAction();
}
