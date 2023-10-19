#include <m2/ui/widget/TextSelection.h>
#include <m2/Game.h>
#include <SDL2/SDL_image.h>

using namespace m2::ui;
using namespace m2::ui::widget;

namespace {
	void throw_if_list_has_duplicates(const std::vector<std::string>& list_ref) {
		// Create a copy of the list
		auto list = list_ref;
		// Remove duplicates
		std::sort(list.begin(), list.end());
		auto new_last = std::unique(list.begin(), list.end());
		list.erase(new_last, list.end());
		// Compare sizes
		if (list.size() != list_ref.size()) {
			throw M2ERROR("TextSelection list cannot have duplicates");
		}
	}
}

TextSelection::TextSelection(State* parent, const WidgetBlueprint* blueprint) : Widget(parent, blueprint),
		_list(text_selection_blueprint().initial_list) {
	throw_if_list_has_duplicates(_list);

	// on_create
	if (text_selection_blueprint().on_create) {
		auto opt_list = text_selection_blueprint().on_create(*this);
		if (opt_list) {
			// Save new list
			_list = *opt_list;
			// Verify list
			throw_if_list_has_duplicates(_list);
		}
	}

	// Select default item
	select(0);
}

Action TextSelection::on_update() {
	if (text_selection_blueprint().on_update) {
		auto[action, optional_list] = text_selection_blueprint().on_update(*this);
		if (action == Action::CONTINUE && optional_list) {
			// Save new list
			_list = *optional_list;
			// Verify list
			throw_if_list_has_duplicates(*optional_list);
			// Select default item
			select(0);
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}

Action TextSelection::on_event(Events& events) {
	auto rect = RectI{rect_px};
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);

	if (!_inc_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
		_inc_depressed = true;
		_dec_depressed = false;
	} else if (!_dec_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = true;
		_inc_depressed = false;
	} else if (_inc_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
		_inc_depressed = false;
		if (_selection + 1 < _list.size()) {
			return select(_selection + 1);
		}
	} else if (_dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = false;
		if (0 < _selection) {
			return select(_selection - 1);
		}
	} else {
		// Check if scrolled
		if (auto scroll_amount = events.pop_mouse_wheel_vertical_scroll(rect); 0 < scroll_amount) {
			auto min_scroll_amount = std::min(static_cast<size_t>(scroll_amount), _list.size() - _selection - 1);
			if (min_scroll_amount) {
				return select(_selection + min_scroll_amount);
			}
		} else if (scroll_amount < 0) {
			auto min_scroll_amount = std::min(static_cast<unsigned>(-scroll_amount), _selection);
			if (min_scroll_amount) {
				return select(_selection - min_scroll_amount);
			}
		}
	}
	return Action::CONTINUE;
}

Action TextSelection::select(unsigned index) {
	_selection = index;
	if (!_list.empty()) {
		_font_texture = *sdl::FontTexture::create(_list[_selection]);

		if (text_selection_blueprint().on_action) {
			return text_selection_blueprint().on_action(*this);
		}
	} else {
		_font_texture = *sdl::FontTexture::create("<EMPTY>");
	}
	return Action::CONTINUE;
}

void TextSelection::on_draw() {
	auto rect = RectI{rect_px};
	auto text_rect = rect.trim_right(rect.h / 2).trim((int) blueprint->padding_width_px);
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
