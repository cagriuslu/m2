#include <m2/ui/element/ButtonDynamicImage.h>
#include <m2/ui/ElementBlueprint.h>

m2::ui::element::ButtonDynamicImageState::ButtonDynamicImageState(const ElementBlueprint* blueprint) : ElementState(blueprint), depressed(false), texture_rect() {}

void m2::ui::element::ButtonDynamicImageState::update_content() {
    texture_rect = std::get<ButtonDynamicImageBlueprint>(blueprint->blueprint_variant).callback();
}

void m2::ui::element::ButtonDynamicImageState::set_depressed(bool state) {
    depressed = state;
}

int m2::ui::element::ButtonDynamicImageState::get_button_return_value() {
    return std::get<ButtonDynamicImageBlueprint>(blueprint->blueprint_variant).return_value;
}

bool m2::ui::element::ButtonDynamicImageState::get_keyboard_shortcut_active(const uint8_t* raw_keyboard_state) const {
    return check_key_pressed(raw_keyboard_state, std::get<ButtonDynamicImageBlueprint>(blueprint->blueprint_variant).keyboard_shortcut);
}
