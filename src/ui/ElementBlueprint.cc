#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

std::unique_ptr<m2::ui::ElementState> m2::ui::ElementBlueprint::get_state() const {
    using namespace element;

    std::unique_ptr<ElementState> state;
    if (std::holds_alternative<NestedUIBlueprint>(blueprint_variant)) {
        state = std::make_unique<NestedUIState>(this);
    } else if (std::holds_alternative<StaticTextBlueprint>(blueprint_variant)) {
        state = std::make_unique<StaticTextState>(this);
    } else if (std::holds_alternative<StaticImageBlueprint>(blueprint_variant)) {
        state = std::make_unique<StaticImageState>(this);
    } else if (std::holds_alternative<DynamicTextBlueprint>(blueprint_variant)) {
        state = std::make_unique<DynamicTextState>(this);
    } else if (std::holds_alternative<DynamicImageBlueprint>(blueprint_variant)) {
        state = std::make_unique<DynamicImageState>(this);
    } else if (std::holds_alternative<ButtonStaticTextBlueprint>(blueprint_variant)) {
        state = std::make_unique<ButtonStaticTextState>(this);
    } else if (std::holds_alternative<ButtonStaticImageBlueprint>(blueprint_variant)) {
        state = std::make_unique<ButtonStaticImageState>(this);
    } else if (std::holds_alternative<ButtonDynamicTextBlueprint>(blueprint_variant)) {
        state = std::make_unique<ButtonDynamicTextState>(this);
    } else if (std::holds_alternative<ButtonDynamicImageBlueprint>(blueprint_variant)) {
        state = std::make_unique<ButtonDynamicImageState>(this);
    }

    return state;
}
