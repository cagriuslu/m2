#include <m2/ui/widget/IntegerSelection.h>
#include <m2/Game.h>
#include <m2/sdl/Font.h>
#include <SDL2/SDL_image.h>

using namespace m2::ui;
using namespace m2::ui::widget;

IntegerSelection::IntegerSelection(State* parent, const WidgetBlueprint *blueprint) : Widget(parent, blueprint),
		_value(std::get<IntegerSelectionBlueprint>(blueprint->variant).initial_value),
		_font_texture(std::move(*sdl::FontTexture::create(std::to_string(_value)))) {
	// Execute on_create
	if (integer_selection_blueprint().on_create) {
		auto opt_value = integer_selection_blueprint().on_create(*this);
		if (opt_value) {
			// Save new value
			_value = *opt_value;
			_font_texture = std::move(*sdl::FontTexture::create(std::to_string(_value)));
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
		if (_value < integer_selection.max_value) {
			select(_value + 1);
		}
	} else if (_dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = false;
		if (integer_selection.min_value < _value) {
			select(_value - 1);
		}
	}
	return Action::CONTINUE;
}

Action IntegerSelection::select(int v) {
	_value = v;
	_font_texture = std::move(*sdl::FontTexture::create(std::to_string(_value)));

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
			_value = *optional_value;
			_font_texture = std::move(*sdl::FontTexture::create(std::to_string(_value)));
		}
	}
	return Action::CONTINUE;
}

void IntegerSelection::on_draw() {
	auto rect = RectI{rect_px};
	auto text_rect = rect.trim_right(rect.h / 2).trim(blueprint->padding_width_px);
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto inc_button_symbol_rect = inc_button_rect.trim(5);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
	auto dec_button_symbol_rect = dec_button_rect.trim(5);

	draw_background_color(rect_px, blueprint->background_color);

	if (_font_texture) {
		draw_text((SDL_Rect)text_rect, _font_texture.texture(), TextAlignment::LEFT);
	}

	static SDL_Texture* up_symbol = IMG_LoadTexture(GAME.renderer, "resource/up-symbol.svg");
	auto up_dstrect = (SDL_Rect)inc_button_symbol_rect;
	SDL_RenderCopy(GAME.renderer, up_symbol, nullptr, &up_dstrect);
	draw_border((SDL_Rect)inc_button_rect, blueprint->border_width_px);

	static SDL_Texture* down_symbol = IMG_LoadTexture(GAME.renderer, "resource/down-symbol.svg");
	auto down_dstrect = (SDL_Rect)dec_button_symbol_rect;
	SDL_RenderCopy(GAME.renderer, down_symbol, nullptr, &down_dstrect);
	draw_border((SDL_Rect)dec_button_rect, blueprint->border_width_px);

	draw_border(rect_px, blueprint->border_width_px);
}
