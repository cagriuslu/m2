#include <m2/Game.h>
#include <m2/ui/widget/TextListSelection.h>

using namespace m2::ui;
using namespace m2::ui::widget;

TextListSelection::TextListSelection(State* parent, const WidgetBlueprint* blueprint)
    : Widget(parent, blueprint),
      _up_arrow_texture(m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, "^"))),
      _down_arrow_texture(m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, "v"))) {
	prepare_list(text_list_selection_blueprint().initial_list);

	// on_create
	if (text_list_selection_blueprint().on_create) {
		auto opt_list = text_list_selection_blueprint().on_create(*this);
		if (opt_list) {
			prepare_list(*opt_list);
		}
	}
}

Action TextListSelection::on_update() {
	if (text_list_selection_blueprint().on_update) {
		auto [action, optional_list] = text_list_selection_blueprint().on_update(*this);
		if (action == Action::CONTINUE && optional_list) {
			prepare_list(*optional_list);
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}

Action TextListSelection::on_event(Events& events) {
	auto scroll_bar_rect = rect_px.trim_left(rect_px.w - rect_px.h / text_list_selection_blueprint().line_count);
	auto up_arrow_rect = scroll_bar_rect.horizontal_split(text_list_selection_blueprint().line_count, 0);
	auto down_button_rect = scroll_bar_rect.horizontal_split(text_list_selection_blueprint().line_count, text_list_selection_blueprint().line_count - 1);

	// Check scroll buttons
	if (events.pop_mouse_button_press(MouseButton::PRIMARY, up_arrow_rect)) {
		if (0 < _top_index) {
			_top_index--;
		}
	} else if (events.pop_mouse_button_press(MouseButton::PRIMARY, down_button_rect)) {
		if (_top_index + text_list_selection_blueprint().line_count < I(_list.size())) {
			_top_index++;
		}
	} else {
		// Check if scrolled
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
				// If already selected
				if (_list[pressed_item].second) {
					// Deselect
					_list[pressed_item].second = false;
				} else {
					// Clear selection if necessary
					if (not text_list_selection_blueprint().allow_multiple_selection) {
						for (auto& item : _list) {
							item.second = false;
						}
					}
					// Select
					_list[pressed_item].second = true;
					// on_action
					if (text_list_selection_blueprint().on_action) {
						return text_list_selection_blueprint().on_action(*this);
					}
				}
			}
		}
	}

	return Action::CONTINUE;
}

void TextListSelection::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);

	for (auto i = 0; i < text_list_selection_blueprint().line_count; ++i) {
		// If the entry is in window
		if (_top_index + i < I(_list.size())) {
			auto text_rect = rect_px.horizontal_split(text_list_selection_blueprint().line_count, i).trim(blueprint->padding_width_px);
			// If selected
			if (_list[_top_index + i].second) {
				draw_background_color(text_rect, {0, 0, 255, 127});
			}
			// Draw text
			draw_text(text_rect, *_list[_top_index + i].first.texture(), TextAlignment::LEFT);
		}
	}

	auto scroll_bar_rect = rect_px.trim_left(rect_px.w - rect_px.h / text_list_selection_blueprint().line_count);
	draw_background_color(scroll_bar_rect, {0, 0, 0, 255});
	draw_border(scroll_bar_rect, blueprint->border_width_px);

	auto up_arrow_rect = scroll_bar_rect.horizontal_split(text_list_selection_blueprint().line_count, 0);
	draw_text(up_arrow_rect, *_up_arrow_texture.texture(), TextAlignment::CENTER);
	draw_border(up_arrow_rect, blueprint->border_width_px);

	auto down_button_rect = scroll_bar_rect.horizontal_split(text_list_selection_blueprint().line_count, text_list_selection_blueprint().line_count - 1);
	draw_text(down_button_rect, *_down_arrow_texture.texture(), TextAlignment::CENTER);
	draw_border(down_button_rect, blueprint->border_width_px);

	draw_border(rect_px, blueprint->border_width_px);
}

void TextListSelection::prepare_list(const std::vector<std::string>& entries) {
	_list.clear();
	for (const auto& entry : entries) {
		_list.emplace_back(m2_move_or_throw_error(sdl::FontTexture::create(GAME.font, GAME.renderer, entry)), false);
	}
}
