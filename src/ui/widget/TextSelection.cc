#include <m2/ui/widget/TextSelection.h>
#include <m2/Game.h>
#include <SDL2/SDL_image.h>

using namespace m2::ui;
using namespace m2::ui::widget;

namespace {
	void throw_if_list_has_duplicates(const std::vector<std::string>& list_ref) {
		auto list = list_ref;
		std::sort(list.begin(), list.end());
		std::unique(list.begin(), list.end());
		if (list.size() != list_ref.size()) {
			throw M2ERROR("TextSelection list cannot have duplicates");
		}
	}
}

TextSelection::TextSelection(const WidgetBlueprint* blueprint) : Widget(blueprint), list(std::get<TextSelectionBlueprint>(blueprint->variant).initial_list) {
	select(0);
	throw_if_list_has_duplicates(list);
}

Action TextSelection::update_content() {
	auto& text_blueprint = std::get<TextSelectionBlueprint>(blueprint->variant);
	if (text_blueprint.update_callback) {
		auto[action, optional_list] = text_blueprint.update_callback(list, selection);
		if (action == Action::CONTINUE && optional_list) {
			// Check list
			throw_if_list_has_duplicates(*optional_list);

			// Save new list
			list = *optional_list;
			// Select default item
			select(0);
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}

Action TextSelection::handle_events(Events& events) {
	auto rect = RectI{rect_px};
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);

	if (!inc_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = true;
		dec_depressed = false;
	} else if (!dec_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = true;
		inc_depressed = false;
	} else if (inc_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = false;
		if (selection + 1 < list.size()) {
			return select(selection + 1);
		}
	} else if (dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = false;
		if (0 < selection) {
			return select(selection - 1);
		}
	}
	return Action::CONTINUE;
}

Action TextSelection::select(unsigned index) {
	selection = index;
	if (!list.empty()) {
		font_texture = *sdl::FontTexture::create(list[selection]);

		const auto& action_callback = std::get<TextSelectionBlueprint>(blueprint->variant).action_callback;
		if (action_callback) {
			return action_callback(list[selection]);
		}
	} else {
		font_texture = *sdl::FontTexture::create("<EMPTY>");
	}
	return Action::CONTINUE;
}

void TextSelection::draw() {
	auto rect = RectI{rect_px};
	auto text_rect = rect.trim_right(rect.h / 2).trim(blueprint->padding_width_px);
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto inc_button_symbol_rect = inc_button_rect.trim(5);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
	auto dec_button_symbol_rect = dec_button_rect.trim(5);

	draw_background_color(rect_px, blueprint->background_color);

	if (font_texture) {
		draw_text((SDL_Rect)text_rect, font_texture.texture(), TextAlignment::LEFT);
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
