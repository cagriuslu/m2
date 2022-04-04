#include <m2/ui/element/ButtonDynamicText.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

m2::ui::element::ButtonDynamicTextState::ButtonDynamicTextState(const ElementBlueprint* blueprint) : ElementState(blueprint), font_texture(nullptr), depressed(false) {}

void m2::ui::element::ButtonDynamicTextState::update_content() {
    // Not yet implemented
    abort();
}

void m2::ui::element::ButtonDynamicTextState::draw() {
    UIState::draw_background_color(rect_px, blueprint->background_color);
    draw_text(rect_px, *font_texture);
    UIState::draw_border(rect_px, blueprint->border_width_px);
}

void m2::ui::element::ButtonDynamicTextState::set_depressed(bool state) {
    depressed = state;
}

int m2::ui::element::ButtonDynamicTextState::get_button_return_value() {
    return std::get<ButtonDynamicTextBlueprint>(blueprint->blueprint_variant).return_value;
}

bool m2::ui::element::ButtonDynamicTextState::get_keyboard_shortcut_active(const uint8_t* raw_keyboard_state) const {
    return check_key_pressed(raw_keyboard_state, std::get<ButtonDynamicTextBlueprint>(blueprint->blueprint_variant).keyboard_shortcut);
}
