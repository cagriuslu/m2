#include <m2/ui/element/StaticText.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

m2::ui::element::StaticTextState::StaticTextState(const ElementBlueprint* blueprint) : ElementState(blueprint), font_texture(nullptr) {}

void m2::ui::element::StaticTextState::update_content() {
    if (!font_texture) {
        auto text = std::get<StaticTextBlueprint>(blueprint->blueprint_variant).text.data();
        font_texture = generate_font_texture(text);
    }
}

void m2::ui::element::StaticTextState::draw() {
    UIState::draw_background_color(rect_px, blueprint->background_color);
    draw_text(rect_px, *font_texture);
    UIState::draw_border(rect_px, blueprint->border_width_px);
}
