#include <m2/ui/element/TextInput.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/ui/UI.h>

using namespace m2::ui;
using namespace m2::ui::element;

TextInputState::TextInputState(const ElementBlueprint* blueprint) : ElementState(blueprint), font_texture(nullptr) {
    text_input << std::get<TextInputBlueprint>(blueprint->variant).initial_text;
    SDL_StartTextInput();
}

TextInputState::~TextInputState() {
    if (font_texture) {
        SDL_DestroyTexture(font_texture);
    }
    SDL_StopTextInput();
}

Action TextInputState::handle_events(Events& events) {
    if (events.pop_key_press(Key::MENU)) {
        return Action::RETURN;
    } else if (events.pop_key_press(Key::ENTER)) {
        return std::get<TextInputBlueprint>(blueprint->variant).action_callback(text_input);
    } else {
		auto opt_text_input = events.pop_text_input();
		if (opt_text_input) {
			text_input << *opt_text_input;
		}
    }
    return Action::CONTINUE;
}

Action TextInputState::update_content() {
    auto new_str = text_input.str();
    if (new_str != font_texture_str) {
        if (font_texture) {
            SDL_DestroyTexture(font_texture);
        }
        font_texture = generate_font_texture(new_str.c_str());
        font_texture_str = new_str;
    }
    return Action::CONTINUE;
}

void TextInputState::draw() {
    UIState::draw_background_color(rect_px, blueprint->background_color);
    draw_text(rect_px, *font_texture, TextAlignment::LEFT);
    UIState::draw_border(rect_px, blueprint->border_width_px);
}
