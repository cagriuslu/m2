#include <m2/ui/widget/TextInput.h>
#include <m2/Log.h>

using namespace m2::ui;
using namespace m2::ui::widget;

namespace {
	void reset_focus() {
		LOG_DEBUG("Stopping text input");
		SDL_StopTextInput();
	}
}

TextInput::TextInput(const WidgetBlueprint* blueprint) : Widget(blueprint) {
	const auto& te_blueprint = std::get<TextInputBlueprint>(blueprint->variant);
	text_input << te_blueprint.initial_text;
}

Action TextInput::handle_events(Events& events) {
	if (events.pop_mouse_button_press(MouseButton::PRIMARY, RectI{rect_px})) {
		LOG_INFO("Regaining focus");
		return Action::GAIN_FOCUS;
	}
	// Ignore all events if not focused
	if (!focused) {
		return Action::CONTINUE;
	}

	if (events.pop_key_press(Key::MENU)) {
		return Action::LOSE_FOCUS;
	} else if (events.pop_key_press(Key::ENTER) && std::get<TextInputBlueprint>(blueprint->variant).action_callback) {
		return std::get<TextInputBlueprint>(blueprint->variant).action_callback(text_input);
	} else if (events.pop_key_press(Key::BACKSPACE)) {
		if (auto text_input_str = text_input.str(); not text_input_str.empty()) {
			text_input = std::stringstream{text_input_str.substr(0, text_input_str.length() - 1)};
			text_input.seekp(0, std::ios::end);
		}
	} else {
		if (auto opt_text_input = events.pop_text_input(); opt_text_input) {
			text_input << *opt_text_input;
		}
	}
	return Action::CONTINUE;
}

void TextInput::focus_changed() {
	if (focused) {
		LOG_DEBUG("Starting text input");
		SDL_StartTextInput();
	} else {
		reset_focus();
	}
}

Action TextInput::update_content() {
	auto str = text_input.str();

	// Add '_' if focused
	if (focused) {
		str += '_';
	}

	// Generate new texture is the string has changed
	if (str != font_texture_str) {
		font_texture = sdl::generate_font(str);
		font_texture_str = str;
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
