#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/UI.h>
#include <m2/Exception.h>

std::unique_ptr<m2::ui::WidgetState> m2::ui::WidgetBlueprint::get_state() const {
    using namespace wdg;

    std::unique_ptr<WidgetState> state;
    if (std::holds_alternative<NestedUIBlueprint>(variant)) {
        state = std::make_unique<NestedUIState>(this);
    } else if (std::holds_alternative<TextBlueprint>(variant)) {
        state = std::make_unique<TextState>(this);
    } else if (std::holds_alternative<TextInputBlueprint>(variant)) {
        state = std::make_unique<TextInputState>(this);
    } else if (std::holds_alternative<ImageBlueprint>(variant)) {
	    state = std::make_unique<ImageState>(this);
    } else if (std::holds_alternative<ProgressBarBlueprint>(variant)) {
	    state = std::make_unique<ProgressBarState>(this);
    } else {
		throw M2FATAL("Implementation");
	}
    return state;
}
