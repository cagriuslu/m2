#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

std::unique_ptr<m2::ui::ElementState> m2::ui::ElementBlueprint::get_state() const {
    using namespace element;

    std::unique_ptr<ElementState> state;
    if (std::holds_alternative<NestedUIBlueprint>(variant)) {
        state = std::make_unique<NestedUIState>(this);
    } else if (std::holds_alternative<TextBlueprint>(variant)) {
        state = std::make_unique<TextState>(this);
    } else if (std::holds_alternative<ImageBlueprint>(variant)) {
        state = std::make_unique<ImageState>(this);
    }

    return state;
}
