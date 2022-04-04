#include <m2/ui/element/ButtonStaticImage.h>
#include <m2/ui/ElementBlueprint.h>

m2::ui::element::ButtonStaticImageState::ButtonStaticImageState(const ElementBlueprint* blueprint) : ElementState(blueprint), depressed(false) {}

void m2::ui::element::ButtonStaticImageState::set_depressed(bool state) {
    depressed = state;
}

int m2::ui::element::ButtonStaticImageState::get_button_return_value() {
    return std::get<ButtonStaticImageBlueprint>(blueprint->blueprint_variant).return_value;
}

bool m2::ui::element::ButtonStaticImageState::get_keyboard_shortcut_active(const uint8_t* raw_keyboard_state) const {
    return check_key_pressed(raw_keyboard_state, std::get<ButtonStaticImageBlueprint>(blueprint->blueprint_variant).keyboard_shortcut);
}
