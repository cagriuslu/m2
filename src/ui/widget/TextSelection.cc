#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>

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
}  // namespace

TextSelection::TextSelection(State* parent, const WidgetBlueprint* blueprint)
    : Widget(parent, blueprint),
      _list(text_selection_blueprint().initial_list),
      _plus_texture(m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, "+"))),
      _minus_texture(m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, "-"))) {
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
		auto [action, optional_list] = text_selection_blueprint().on_update(*this);
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
	auto buttons_rect = rect_px.trim_left(rect_px.w - rect_px.h / 2);
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
		if (auto scroll_amount = events.pop_mouse_wheel_vertical_scroll(rect_px); 0 < scroll_amount) {
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

void TextSelection::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);

	if (auto* texture = _font_texture.texture(); texture) {
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

Action TextSelection::select(unsigned index) {
	_selection = index;
	if (!_list.empty()) {
		_font_texture = m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, _list[_selection]));

		if (text_selection_blueprint().on_action) {
			return text_selection_blueprint().on_action(*this);
		}
	} else {
		_font_texture = m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, "<EMPTY>"));
	}
	return Action::CONTINUE;
}
