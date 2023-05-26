#include <m2/ui/widget/IntegerSelection.h>
#include <m2/Game.h>
#include <SDL2/SDL_image.h>

using namespace m2::ui;
using namespace m2::ui::widget;

IntegerSelection::IntegerSelection(const WidgetBlueprint *blueprint) : Widget(blueprint), value(std::get<IntegerSelectionBlueprint>(blueprint->variant).initial_value), font_texture(sdl::generate_font(std::to_string(value))) {}

Action IntegerSelection::handle_events(Events& events) {
	auto rect = RectI{rect_px};
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);

	const auto& integer_selection = std::get<IntegerSelectionBlueprint>(blueprint->variant);

	if (!inc_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = true;
		dec_depressed = false;
	} else if (!dec_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = true;
		inc_depressed = false;
	} else if (inc_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = false;
		if (value < integer_selection.max_value) {
			select(value + 1);
		}
	} else if (dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = false;
		if (integer_selection.min_value < value) {
			select(value - 1);
		}
	}
	return Action::CONTINUE;
}

Action IntegerSelection::select(int _value) {
	value = _value;
	font_texture = sdl::generate_font(std::to_string(value));

	const auto& integer_selection = std::get<IntegerSelectionBlueprint>(blueprint->variant);
	const auto& action_callback = integer_selection.action_callback;
	if (action_callback) {
		return action_callback(value);
	}
	return Action::CONTINUE;
}

Action IntegerSelection::update_content() {
	auto& pb_blueprint = std::get<IntegerSelectionBlueprint>(blueprint->variant);
	if (pb_blueprint.update_callback) {
		auto optional_value = pb_blueprint.update_callback();
		if (optional_value) {
			value = *optional_value;
			font_texture = sdl::generate_font(std::to_string(value));
		}
	}
	return Action::CONTINUE;
}

void IntegerSelection::draw() {
	auto rect = RectI{rect_px};
	auto text_rect = rect.trim_right(rect.h / 2).trim(blueprint->padding_width_px);
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto inc_button_symbol_rect = inc_button_rect.trim(5);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
	auto dec_button_symbol_rect = dec_button_rect.trim(5);

	draw_background_color(rect_px, blueprint->background_color);

	if (font_texture) {
		draw_text((SDL_Rect)text_rect, *font_texture, TextAlignment::LEFT);
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
