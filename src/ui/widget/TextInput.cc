#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/sdl/FontTexture.h>
#include <m2/ui/widget/TextInput.h>

using namespace m2::ui;
using namespace m2::ui::widget;

namespace {
	void reset_focus() {
		LOG_DEBUG("Stopping text input");
		SDL_StopTextInput();
	}
}  // namespace

TextInput::TextInput(Panel* parent, const WidgetBlueprint* blueprint) : Widget(parent, blueprint) {
	const auto& te_blueprint = std::get<TextInputBlueprint>(blueprint->variant);
	_text_input << te_blueprint.initial_text;
}

Action TextInput::on_event(Events& events) {
	if (events.pop_mouse_button_press(MouseButton::PRIMARY, RectI{rect()})) {
		LOG_INFO("Regaining focus");
		return make_continue_action(true);
	}
	// Ignore all events if not focused
	if (!focused) {
		return make_continue_action();
	}

	if (events.pop_key_press(Key::MENU)) {
		return make_continue_action(false);
	} else if (events.pop_key_press(Key::ENTER) && std::get<TextInputBlueprint>(blueprint->variant).on_action) {
		auto [action, new_string] = std::get<TextInputBlueprint>(blueprint->variant).on_action(*this);
		if (new_string) {
			_text_input = std::stringstream{*new_string};
		}
		return std::move(action);
	} else if (events.pop_key_press(Key::BACKSPACE)) {
		if (const auto text_input_str = _text_input.str(); not text_input_str.empty()) {
			_text_input = std::stringstream{text_input_str.substr(0, text_input_str.length() - 1)};
			_text_input.seekp(0, std::ios::end);
		}
	} else {
		if (const auto opt_text_input = events.pop_text_input(); opt_text_input) {
			_text_input << *opt_text_input;
		}
	}
	return make_continue_action();
}

void TextInput::on_focus_change() {
	if (focused) {
		LOG_DEBUG("Starting text input");
		SDL_StartTextInput();
	} else {
		reset_focus();
	}
}

Action TextInput::on_update() {
	auto str = _text_input.str();

	// Add '_' if focused
	if (focused) {
		str += '_';
	}

	// Generate new texture is the string has changed
	if (not _font_texture || str != _font_texture.string()) {
		_font_texture = m2_move_or_throw_error(sdl::FontTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, str));
	}

	return make_continue_action();
}

void TextInput::on_draw() {
	draw_background_color();
	if (const auto texture = _font_texture.texture(); texture) {
		sdl::render_texture_with_color_mod(texture, calculate_text_rect(texture, rect(), TextHorizontalAlignment::LEFT));
	}
	draw_border(rect(), vertical_border_width_px(), horizontal_border_width_px());
}
