#include <m2/ui/widget/TextInput.h>

using namespace m2::ui;
using namespace m2::ui::widget;

TextInput::TextInput(const WidgetBlueprint* blueprint) : Widget(blueprint), font_texture(nullptr) {
	text_input << std::get<TextInputBlueprint>(blueprint->variant).initial_text;
	SDL_StartTextInput();
}

TextInput::~TextInput() {
	SDL_StopTextInput();
}

Action TextInput::handle_events(Events& events) {
	if (events.pop_key_press(Key::MENU)) {
		return Action::RETURN;
	} else if (events.pop_key_press(Key::ENTER)) {
		return std::get<TextInputBlueprint>(blueprint->variant).action_callback(text_input);
	} else if (events.pop_key_press(Key::BACKSPACE)) {
		auto text_input_str = text_input.str();
		if (not text_input_str.empty()) {
			text_input = std::stringstream{text_input_str.substr(0, text_input_str.length() - 1)};
			text_input.seekp(0, std::ios::end);
		}
	} else {
		auto opt_text_input = events.pop_text_input();
		if (opt_text_input) {
			text_input << *opt_text_input;
		}
	}
	return Action::CONTINUE;
}

Action TextInput::update_content() {
	auto new_str = text_input.str() + '_';
	if (new_str != font_texture_str) {
		font_texture = sdl::generate_font(new_str);
		font_texture_str = new_str;
	}
	return Action::CONTINUE;
}

void TextInput::draw() {
	draw_background_color(rect_px, blueprint->background_color);
	if (font_texture) {
		draw_text(rect_px, *font_texture, TextAlignment::LEFT);
	}
	draw_border(rect_px, blueprint->border_width_px);
}
