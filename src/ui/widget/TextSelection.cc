#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

namespace {
	std::vector<sdl::FontTexture> generate_option_texts(const TextSelectionBlueprint::Options& options) {
		std::vector<sdl::FontTexture> texts;
		for (const auto& option : options) {
			texts.emplace_back(m2_move_or_throw_error(sdl::FontTexture::create(M2_GAME.font, M2_GAME.renderer, option.first)));
		}
		return texts;
	}
}

TextSelection::TextSelection(State* parent, const WidgetBlueprint* blueprint) : Widget(parent, blueprint),
	_plus_texture(m2_move_or_throw_error(sdl::FontTexture::create(M2_GAME.font, M2_GAME.renderer, "+"))),
	_minus_texture(m2_move_or_throw_error(sdl::FontTexture::create(M2_GAME.font, M2_GAME.renderer, "-"))),
	_up_arrow_texture(m2_move_or_throw_error(sdl::FontTexture::create(M2_GAME.font, M2_GAME.renderer, "^"))),
	_down_arrow_texture(m2_move_or_throw_error(sdl::FontTexture::create(M2_GAME.font, M2_GAME.renderer, "v"))) {
	reset();
}

Action TextSelection::on_update() {
	if (text_list_selection_blueprint().on_update) {
		return text_list_selection_blueprint().on_update(*this);
	}
	return make_continue_action();
}

Action TextSelection::on_event(Events& events) {
	// +/- selection
	if (auto line_count = text_list_selection_blueprint().line_count; line_count == 0) {
		auto buttons_rect = rect_px.trim_left(rect_px.w - rect_px.h / 2);
		auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
		auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
		if (!_plus_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
			_plus_depressed = true;
			_minus_depressed = false;
		} else if (!_minus_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
			_minus_depressed = true;
			_plus_depressed = false;
		} else if (_plus_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
			_plus_depressed = false;
			return increment_selection(1);
		} else if (_minus_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
			_minus_depressed = false;
			return decrement_selection(1);
		} else {
			// Check if scrolled
			if (auto scroll_amount = events.pop_mouse_wheel_vertical_scroll(rect_px); 0 < scroll_amount) {
				return increment_selection(scroll_amount);
			} else if (scroll_amount < 0) {
				return decrement_selection(scroll_amount);
			}
		}
	} else if (line_count == 1) {
		// Dropdown
		throw M2ERROR("Not yet implemented");
	} else {
		// Scrollable selection
		auto scroll_bar_rect = rect_px.trim_left(rect_px.w - rect_px.h / I(text_list_selection_blueprint().line_count));
		auto up_arrow_rect = scroll_bar_rect.horizontal_split(I(text_list_selection_blueprint().line_count), 0);
		auto down_button_rect = scroll_bar_rect.horizontal_split(I(text_list_selection_blueprint().line_count),
			I(text_list_selection_blueprint().line_count) - 1);

		// Check if scroll buttons are pressed
		if (events.pop_mouse_button_press(MouseButton::PRIMARY, up_arrow_rect)) {
			if (0 < _top_index) {
				_top_index--;
			}
		} else if (events.pop_mouse_button_press(MouseButton::PRIMARY, down_button_rect)) {
			if (_top_index + text_list_selection_blueprint().line_count < I(_list.size())) {
				_top_index++;
			}
		} else {
			// Check if scrolled via mouse
			if (auto scroll_amount = events.pop_mouse_wheel_vertical_scroll(rect_px); 0 < scroll_amount) {
				auto min_scroll_amount = std::min(static_cast<size_t>(scroll_amount), _list.size() - _top_index - text_list_selection_blueprint().line_count);
				if (min_scroll_amount) {
					_top_index += I(min_scroll_amount);
				}
			} else if (scroll_amount < 0) {
				auto min_scroll_amount = std::min(-scroll_amount, _top_index);
				if (min_scroll_amount) {
					_top_index -= min_scroll_amount;
				}
			}
		}

		// Check line items
		for (auto i = 0; i < text_list_selection_blueprint().line_count; ++i) {
			// If the entry is in window
			if (_top_index + i < I(_list.size())) {
				auto text_rect = rect_px.horizontal_split(text_list_selection_blueprint().line_count, i).trim_right(scroll_bar_rect.w);
				if (events.pop_mouse_button_press(MouseButton::PRIMARY, text_rect)) {
					int pressed_item = _top_index + i;
					if (_selections[pressed_item]) {
						// If already selected
						_selections[pressed_item] = false; // Deselect
						if (text_list_selection_blueprint().on_action) {
							return text_list_selection_blueprint().on_action(*this);
						}
					} else {
						// Clear selection if necessary
						if (not text_list_selection_blueprint().allow_multiple_selection) {
							_selections = std::vector(_list.size(), false);
						}
						_selections[pressed_item] = true; // Select
						if (text_list_selection_blueprint().on_action) {
							return text_list_selection_blueprint().on_action(*this);
						}
					}
				}
			}
		}
	}
	return make_continue_action();
}

void TextSelection::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);

	// +/- selection
	if (auto line_count = text_list_selection_blueprint().line_count; line_count == 0) {
		if (auto current_selection = std::find(_selections.begin(), _selections.end(), true); current_selection != _selections.end()) {
			auto current_selection_index = std::distance(_selections.begin(), current_selection);
			draw_text(calculate_text_rect(rect_px.trim_right(rect_px.h / 2), blueprint->padding_width_px, 0, 0, TextAlignment::LEFT,
				_option_texts[current_selection_index].texture()), _option_texts[current_selection_index].texture());
		}

		// + button
		auto buttons_rect = rect_px.trim_left(rect_px.w - rect_px.h / 2);
		auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
		draw_text(
			calculate_text_rect(inc_button_rect, 0, 0, 0, TextAlignment::CENTER, _plus_texture.texture()),
			_plus_texture.texture());
		draw_border(inc_button_rect, blueprint->border_width_px);

		// - button
		auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
		draw_text(
			calculate_text_rect(dec_button_rect, 0, 0, 0, TextAlignment::CENTER, _minus_texture.texture()),
			_minus_texture.texture());
		draw_border(dec_button_rect, blueprint->border_width_px);
	} else if (line_count == 1) {
		// Dropdown
		throw M2ERROR("Not yet implemented");
	} else {
		// Scrollable selection
		for (auto i = 0; i < text_list_selection_blueprint().line_count; ++i) {
			// If the entry is in window
			if (_top_index + i < I(_list.size())) {
				auto text_rect = rect_px.horizontal_split(text_list_selection_blueprint().line_count, i);
				// If selected
				if (_selections[_top_index + i]) {
					draw_background_color(text_rect, {0, 0, 255, 127});
				}
				// Draw text
				auto texture = _option_texts[_top_index + i].texture();
				draw_text(
					calculate_text_rect(text_rect, blueprint->padding_width_px, 0, 0, TextAlignment::LEFT, texture),
					texture);
			}
		}

		if (text_list_selection_blueprint().show_scroll_bar) {
			auto scroll_bar_rect = rect_px.trim_left(rect_px.w - rect_px.h / text_list_selection_blueprint().line_count);
			draw_background_color(scroll_bar_rect, {0, 0, 0, 255});
			draw_border(scroll_bar_rect, blueprint->border_width_px);

			auto up_arrow_rect = scroll_bar_rect.horizontal_split(text_list_selection_blueprint().line_count, 0);
			draw_text(
				calculate_text_rect(up_arrow_rect, 0, 0, 0, TextAlignment::CENTER, _up_arrow_texture.texture()),
				_up_arrow_texture.texture());
			draw_border(up_arrow_rect, blueprint->border_width_px);

			auto down_button_rect = scroll_bar_rect.horizontal_split(
				text_list_selection_blueprint().line_count, text_list_selection_blueprint().line_count - 1);
			draw_text(
				calculate_text_rect(down_button_rect, 0, 0, 0, TextAlignment::CENTER, _down_arrow_texture.texture()),
				_down_arrow_texture.texture());
			draw_border(down_button_rect, blueprint->border_width_px);
		}
	}

	draw_border(rect_px, blueprint->border_width_px);
}

std::vector<TextSelectionBlueprint::ValueVariant> TextSelection::selections() const {
	std::vector<TextSelectionBlueprint::ValueVariant> selections;
	for (size_t i = 0; i < _selections.size(); ++i) {
		if (_selections[i]) {
			selections.emplace_back(_list[i].second);
		}
	}
	return selections;
}

void TextSelection::reset() {
	set_options(text_list_selection_blueprint().initial_list);

	if (text_list_selection_blueprint().on_create) {
		text_list_selection_blueprint().on_create(*this);
	}
}

void TextSelection::set_options(TextSelectionBlueprint::Options options) {
	_list = std::move(options);
	_option_texts = generate_option_texts(_list);
	_selections = std::vector<bool>(_list.size(), false);

	// Applicable to +/- selection and dropdown
	// Select the first item
	if (auto line_count = text_list_selection_blueprint().line_count; line_count == 0 || line_count == 1) {
		if (not _selections.empty()) {
			_selections[0] = true;
			if (text_list_selection_blueprint().on_action) {
				text_list_selection_blueprint().on_action(*this);
			}
		}
	}
}

void TextSelection::set_selection(int index) {
	if (0 <= index && index < I(_selections.size())) {
		_selections = std::vector<bool>(_list.size(), false);
		_selections[index] = true;
		if (text_list_selection_blueprint().on_action) {
			text_list_selection_blueprint().on_action(*this);
		}
	}
}

const TextSelectionBlueprint& TextSelection::text_list_selection_blueprint() const {
	return std::get<TextSelectionBlueprint>(blueprint->variant);
}

Action TextSelection::increment_selection(int count) {
	if (auto current_selection = std::find(_selections.begin(), _selections.end(), true); current_selection != _selections.end()) {
		auto current_selection_index = std::distance(_selections.begin(), current_selection);
		for (int i = 0; i < count && current_selection_index + 1 < I(_selections.size()); ++i) {
			_selections[current_selection_index++] = false;
			_selections[current_selection_index] = true;
		}
		if (text_list_selection_blueprint().on_action) {
			return text_list_selection_blueprint().on_action(*this);
		}
	}
	return make_continue_action();
}

Action TextSelection::decrement_selection(int count) {
	if (auto current_selection = std::find(_selections.begin(), _selections.end(), true); current_selection != _selections.end()) {
		auto current_selection_index = std::distance(_selections.begin(), current_selection);
		for (int i = 0; i < count && 0 < current_selection_index; ++i) {
			_selections[current_selection_index--] = false;
			_selections[current_selection_index] = true;
		}
		if (text_list_selection_blueprint().on_action) {
			return text_list_selection_blueprint().on_action(*this);
		}
	}
	return make_continue_action();
}
