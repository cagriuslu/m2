#include <m2/ui/element/ButtonStaticText.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

m2::ui::element::ButtonStaticTextState::ButtonStaticTextState(const ElementBlueprint* blueprint) : ElementState(blueprint), font_texture(nullptr), depressed(false) {}

void m2::ui::element::ButtonStaticTextState::update_content() {
    if (!font_texture) {
        auto text = std::get<ButtonStaticTextBlueprint>(blueprint->blueprint_variant).text.data();
        font_texture = generate_font_texture(text);
    }
}

void m2::ui::element::ButtonStaticTextState::draw() {
    UIState::draw_background_color(rect_px, blueprint->background_color);
    draw_text(rect_px, *font_texture);
    UIState::draw_border(rect_px, blueprint->border_width_px);
}

void m2::ui::element::ButtonStaticTextState::set_depressed(bool state) {
    depressed = state;
}

int m2::ui::element::ButtonStaticTextState::get_button_return_value() {
    return std::get<ButtonStaticTextBlueprint>(blueprint->blueprint_variant).return_value;
}

bool m2::ui::element::ButtonStaticTextState::get_keyboard_shortcut_active(const uint8_t* raw_keyboard_state) const {
    return check_key_pressed(raw_keyboard_state, std::get<ButtonStaticTextBlueprint>(blueprint->blueprint_variant).keyboard_shortcut);
}
