#include <m2/ui/element/DynamicText.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

m2::ui::element::DynamicTextState::DynamicTextState(const ElementBlueprint* blueprint) : ElementState(blueprint), font_texture(nullptr) {}

void m2::ui::element::DynamicTextState::update_content() {
    auto string = std::get<DynamicTextBlueprint>(blueprint->blueprint_variant).callback();
    if (font_texture) {
        SDL_DestroyTexture(font_texture);
    }
    font_texture = generate_font_texture(string.c_str());
}

void m2::ui::element::DynamicTextState::draw() {
    UIState::draw_background_color(rect_px, blueprint->background_color);
    draw_text(rect_px, *font_texture);
    UIState::draw_border(rect_px, blueprint->border_width_px);
}
