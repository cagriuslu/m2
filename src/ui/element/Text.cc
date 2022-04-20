#include <m2/ui/element/Text.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

m2::ui::element::TextState::TextState(const ElementBlueprint* blueprint) : AbstractButtonState(blueprint), font_texture(generate_font_texture(std::get<TextBlueprint>(blueprint->variant).initial_text.data())) {}

m2::ui::element::TextState::~TextState() {
	SDL_DestroyTexture(font_texture);
}

m2::ui::Action m2::ui::element::TextState::update_content() {
    auto& text_blueprint = std::get<TextBlueprint>(blueprint->variant);
    if (text_blueprint.update_callback) {
        auto[action, optional_string] = text_blueprint.update_callback();
        if (action == Action::CONTINUE && optional_string) {
            if (font_texture) {
                SDL_DestroyTexture(font_texture);
            }
            font_texture = generate_font_texture(optional_string->c_str());
        }
        return action;
    } else {
        return Action::CONTINUE;
    }
}

void m2::ui::element::TextState::draw() {
    UIState::draw_background_color(rect_px, blueprint->background_color);
    draw_text(rect_px, *font_texture);
    UIState::draw_border(rect_px, blueprint->border_width_px);
}
