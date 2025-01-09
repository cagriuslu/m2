#include <m2/Game.h>
#include <m2/sdl/TextTexture.h>
#include <m2/ui/widget/IntegerInput.h>

using namespace m2;
using namespace m2::widget;

IntegerInput::IntegerInput(UiPanel* parent, const UiWidgetBlueprint* blueprint)
    : UiWidget(parent, blueprint), _value(std::get<IntegerInputBlueprint>(blueprint->variant).initial_value),
      _plus_texture(m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, "+"))),
      _minus_texture(m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, "-"))) {
	_textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, ToString(_value)));

	// Execute on_create
	if (integer_selection_blueprint().on_create) {
		auto opt_value = integer_selection_blueprint().on_create(*this);
		if (opt_value) {
			// Save new value
			_value = *opt_value;
			_textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, ToString(*opt_value)));
		}
	}
}

UiAction IntegerInput::on_event(Events& events) {
	auto buttons_rect = rect().trim_left(rect().w - rect().h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);

	const auto& integer_selection = std::get<IntegerInputBlueprint>(blueprint->variant);

	if (!_inc_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
		_inc_depressed = true;
		_dec_depressed = false;
	} else if (!_dec_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = true;
		_inc_depressed = false;
	} else if (_inc_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
		_inc_depressed = false;
		if (value() < integer_selection.max_value) {
			select(value() + 1);
		}
	} else if (_dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = false;
		if (integer_selection.min_value < value()) {
			select(value() - 1);
		}
	}
	return MakeContinueAction();
}

UiAction IntegerInput::select(int v) {
	_value = v;
	_textTexture = std::move(*sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, ToString(v)));

	const auto& integer_selection = std::get<IntegerInputBlueprint>(blueprint->variant);
	const auto& action_callback = integer_selection.on_action;
	if (action_callback) {
		return action_callback(*this);
	}
	return MakeContinueAction();
}

UiAction IntegerInput::on_update() {
	auto& pb_blueprint = std::get<IntegerInputBlueprint>(blueprint->variant);
	if (pb_blueprint.on_update) {
		auto optional_value = pb_blueprint.on_update(*this);
		if (optional_value) {
			_value = *optional_value;
			_textTexture = std::move(*sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, ToString(*optional_value)));
		}
	}
	return MakeContinueAction();
}

void IntegerInput::on_draw() {
	draw_background_color();

	if (const auto texture = _textTexture.texture(); texture) {
		sdl::render_texture_with_color_mod(texture,
				calculate_filled_text_rect(rect().trim_right(rect().h / 2), TextHorizontalAlignment::LEFT,
					I(utf8_codepoint_count(_textTexture.string().c_str()))));
	}

	auto buttons_rect = rect().trim_left(rect().w - rect().h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	sdl::render_texture_with_color_mod(_plus_texture.texture(),
			calculate_filled_text_rect(inc_button_rect,
				TextHorizontalAlignment::LEFT, I(utf8_codepoint_count(_plus_texture.string().c_str()))));
	draw_border(inc_button_rect, vertical_border_width_px(), horizontal_border_width_px());

	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
	sdl::render_texture_with_color_mod(_minus_texture.texture(),
			calculate_filled_text_rect(dec_button_rect,
				TextHorizontalAlignment::LEFT, I(utf8_codepoint_count(_minus_texture.string().c_str()))));
	draw_border(dec_button_rect, vertical_border_width_px(), horizontal_border_width_px());

	draw_border(rect(), vertical_border_width_px(), horizontal_border_width_px());
}
