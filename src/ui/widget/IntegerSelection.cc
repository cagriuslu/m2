#include <m2/ui/widget/IntegerSelection.h>
#include <m2/Game.h>
#include <m2/sdl/Font.h>

using namespace m2::ui;
using namespace m2::ui::widget;

IntegerSelection::IntegerSelection(State* parent, const WidgetBlueprint *blueprint) : Widget(parent, blueprint),
		_plus_texture(m2_move_or_throw_error(sdl::FontTexture::create("+"))),
		_minus_texture(m2_move_or_throw_error(sdl::FontTexture::create("-"))) {
	const auto inital_value = std::get<IntegerSelectionBlueprint>(blueprint->variant).initial_value;
	_font_texture = m2_move_or_throw_error(sdl::FontTexture::create(inital_value));

	// Execute on_create
	if (integer_selection_blueprint().on_create) {
		auto opt_value = integer_selection_blueprint().on_create(*this);
		if (opt_value) {
			// Save new value
			_font_texture = m2_move_or_throw_error(sdl::FontTexture::create(*opt_value));
		}
	}
}

Action IntegerSelection::on_event(Events& events) {
	auto rect = RectI{rect_px};
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);

	const auto& integer_selection = std::get<IntegerSelectionBlueprint>(blueprint->variant);

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
	return Action::CONTINUE;
}

Action IntegerSelection::select(int v) {
	_font_texture = std::move(*sdl::FontTexture::create(v));

	const auto& integer_selection = std::get<IntegerSelectionBlueprint>(blueprint->variant);
	const auto& action_callback = integer_selection.on_action;
	if (action_callback) {
		return action_callback(*this);
	}
	return Action::CONTINUE;
}

Action IntegerSelection::on_update() {
	auto& pb_blueprint = std::get<IntegerSelectionBlueprint>(blueprint->variant);
	if (pb_blueprint.on_update) {
		auto optional_value = pb_blueprint.on_update(*this);
		if (optional_value) {
			_font_texture = std::move(*sdl::FontTexture::create(*optional_value));
		}
	}
	return Action::CONTINUE;
}

void IntegerSelection::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);

	if (const auto texture = _font_texture.texture(); texture) {
		auto text_rect = rect_px.trim_right(rect_px.h / 2).trim(blueprint->padding_width_px);
		draw_text(text_rect, *texture, TextAlignment::LEFT);
	}

	auto buttons_rect = rect_px.trim_left(rect_px.w - rect_px.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	draw_text(inc_button_rect, *_plus_texture.texture(), TextAlignment::CENTER);
	draw_border(inc_button_rect, blueprint->border_width_px);

	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
	draw_text(dec_button_rect, *_minus_texture.texture(), TextAlignment::CENTER);
	draw_border(dec_button_rect, blueprint->border_width_px);

	draw_border(rect_px, blueprint->border_width_px);
}
