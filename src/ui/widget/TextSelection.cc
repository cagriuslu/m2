#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

TextSelection::TextSelection(Panel* parent, const WidgetBlueprint* blueprint) : Widget(parent, blueprint) {
	set_options(text_list_selection_blueprint().options);
	if (text_list_selection_blueprint().on_create) {
		text_list_selection_blueprint().on_create(*this);
	}
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
		auto buttons_rect = rect().trim_left(rect().w - rect().h / 2);
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
			return increment_selection();
		} else if (_minus_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
			_minus_depressed = false;
			return decrement_selection();
		} else {
			// Check if scrolled
			if (auto scroll_amount = events.pop_mouse_wheel_vertical_scroll(rect()); 0 < scroll_amount) {
				m2_repeat(scroll_amount) {
					if (auto action = increment_selection(); not action.is_continue()) {
						return action;
					}
				}
				return make_continue_action();
			} else if (scroll_amount < 0) {
				m2_repeat(scroll_amount) {
					if (auto action = decrement_selection(); not action.is_continue()) {
						return action;
					}
				}
				return make_continue_action();
			}
		}
	} else if (line_count == 1) {
		// Dropdown
		throw M2_ERROR("Not yet implemented");
	} else {
		// Scrollable selection
		auto scroll_bar_rect = rect().trim_left(rect().w - rect().h / I(text_list_selection_blueprint().line_count));
		auto up_arrow_rect = scroll_bar_rect.horizontal_split(I(text_list_selection_blueprint().line_count), 0);
		auto down_button_rect = scroll_bar_rect.horizontal_split(I(text_list_selection_blueprint().line_count),
			I(text_list_selection_blueprint().line_count) - 1);

		// Check if scroll buttons are pressed
		if (events.pop_mouse_button_press(MouseButton::PRIMARY, up_arrow_rect)) {
			if (0 < _top_index) {
				_top_index--;
			}
		} else if (events.pop_mouse_button_press(MouseButton::PRIMARY, down_button_rect)) {
			if (_top_index + text_list_selection_blueprint().line_count < I(_options.size())) {
				_top_index++;
			}
		} else {
			// Check if scrolled via mouse
			if (auto scroll_amount = events.pop_mouse_wheel_vertical_scroll(rect()); 0 < scroll_amount) {
				auto min_scroll_amount = std::min(static_cast<size_t>(scroll_amount), _options.size() - _top_index - text_list_selection_blueprint().line_count);
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
			if (_top_index + i < I(_options.size())) {
				auto text_rect = rect().horizontal_split(text_list_selection_blueprint().line_count, i).trim_right(scroll_bar_rect.w);
				if (events.pop_mouse_button_press(MouseButton::PRIMARY, text_rect)) {
					int pressed_item = _top_index + i;
					if (_options[pressed_item].is_selected) {
						// If already selected
						_options[pressed_item].is_selected = false; // Deselect
						if (text_list_selection_blueprint().on_action) {
							return text_list_selection_blueprint().on_action(*this);
						}
					} else {
						// Clear selection if necessary
						if (not text_list_selection_blueprint().allow_multiple_selection) {
							// Clear all selections
							for (auto& option : _options) {
								option.is_selected = false;
							}
						}
						_options[pressed_item].is_selected = true; // Select
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
	draw_background_color();

	// +/- selection
	if (auto line_count = text_list_selection_blueprint().line_count; line_count == 0) {
		// Selected option's text
		if (auto current_selection = std::ranges::find_if(_options, [](const auto& o) { return o.is_selected == true; });
			current_selection != _options.end()) {
			if (not current_selection->text_texture_and_destination) {
				auto drawable_area = rect().trim_right(rect().h / 2);
				auto fontSize = calculate_filled_text_rect(drawable_area, TextHorizontalAlignment::LEFT, I(m2::utf8_codepoint_count(current_selection->blueprint_option.text.c_str()))).h;
				auto textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, fontSize, current_selection->blueprint_option.text));
				auto destination_rect = calculate_filled_text_rect(drawable_area, TextHorizontalAlignment::LEFT, I(m2::utf8_codepoint_count(current_selection->blueprint_option.text.c_str())));
				current_selection->text_texture_and_destination = sdl::TextTextureAndDestination{std::move(textTexture), destination_rect};
			}
			sdl::render_texture_with_color_mod(current_selection->text_texture_and_destination->textTexture.texture(),
				current_selection->text_texture_and_destination->destinationRect, current_selection->blueprint_option.text_color);
		}
		// + button
		auto buttons_rect = rect().trim_left(rect().w - rect().h / 2);
		{
			auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
			if (not _plus_texture) {
				auto fontSize = inc_button_rect.h;
				auto textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, fontSize, "+"));
				auto destination_rect = RectI::centered_around(inc_button_rect.center(), textTexture.texture_dimensions().x, textTexture.texture_dimensions().y);
				// TODO we may need to move the texture slightly up, check the font properties
				_plus_texture = {std::move(textTexture), destination_rect};
			}
			sdl::render_texture_with_color_mod(_plus_texture->textTexture.texture(), _plus_texture->destinationRect);
			draw_border(inc_button_rect, vertical_border_width_px(), horizontal_border_width_px());
		}
		// - button
		{
			auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
			if (not _minus_texture) {
				auto fontSize = dec_button_rect.h;
				auto textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, fontSize, "-"));
				auto destination_rect = RectI::centered_around(dec_button_rect.center(), textTexture.texture_dimensions().x, textTexture.texture_dimensions().y);
				// TODO we may need to move the texture slightly up, check the font properties
				_minus_texture = {std::move(textTexture), destination_rect};
			}
			sdl::render_texture_with_color_mod(_minus_texture->textTexture.texture(), _minus_texture->destinationRect);
			draw_border(dec_button_rect, vertical_border_width_px(), horizontal_border_width_px());
		}
	} else if (line_count == 1) {
		// Dropdown
		throw M2_ERROR("Not yet implemented");
	} else {
		// Scrollable selection
		for (auto i = 0; i < text_list_selection_blueprint().line_count; ++i) {
			// If the entry is in window
			if (_top_index + i < I(_options.size())) {
				auto text_rect = rect().horizontal_split(text_list_selection_blueprint().line_count, i);
				// If selected
				if (_options[_top_index + i].is_selected) {
					draw_rectangle(text_rect, {0, 0, 255, 127});
				}
				// Draw text
				auto& current_line = _options[_top_index + i];
				if (not current_line.text_texture_and_destination) {
					auto fontSize = calculate_filled_text_rect(text_rect, TextHorizontalAlignment::LEFT, I(m2::utf8_codepoint_count(current_line.blueprint_option.text.c_str()))).h;
					auto textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, fontSize, current_line.blueprint_option.text));
					// Don't bother with destination_rect, because we're going to calculate that every time
					current_line.text_texture_and_destination = sdl::TextTextureAndDestination{std::move(textTexture), {}};
				}
				// Upon scroll, the destination might still have changed, calculate it again.
				auto destination_rect = calculate_filled_text_rect(text_rect, TextHorizontalAlignment::LEFT, I(m2::utf8_codepoint_count(current_line.blueprint_option.text.c_str())));
				current_line.text_texture_and_destination->destinationRect = destination_rect;
				sdl::render_texture_with_color_mod(current_line.text_texture_and_destination->textTexture.texture(),
					current_line.text_texture_and_destination->destinationRect, current_line.blueprint_option.text_color);
			}
		}
		// Scroll bar
		if (text_list_selection_blueprint().show_scroll_bar) {
			auto scroll_bar_rect = rect().trim_left(rect().w - rect().h / text_list_selection_blueprint().line_count);
			draw_rectangle(scroll_bar_rect, {0, 0, 0, 255});
			draw_border(scroll_bar_rect, vertical_border_width_px(), horizontal_border_width_px());
			// Up arrow
			{
				auto up_arrow_rect = scroll_bar_rect.horizontal_split(text_list_selection_blueprint().line_count, 0);
				if (not _up_arrow_texture) {
					auto fontSize = up_arrow_rect.h;
					auto textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, fontSize, "^"));
					auto destination_rect = RectI::centered_around(up_arrow_rect.center(), textTexture.texture_dimensions().x, textTexture.texture_dimensions().y);
					// TODO we may need to move the texture slightly up, check the font properties
					_up_arrow_texture = {std::move(textTexture), destination_rect};
				}
				sdl::render_texture_with_color_mod(_up_arrow_texture->textTexture.texture(), _up_arrow_texture->destinationRect);
				draw_border(up_arrow_rect, vertical_border_width_px(), horizontal_border_width_px());
			}
			// Down arrow
			{
				auto down_button_rect = scroll_bar_rect.horizontal_split(text_list_selection_blueprint().line_count,
					text_list_selection_blueprint().line_count - 1);
				if (not _down_arrow_texture) {
					auto fontSize = down_button_rect.h;
					auto textTexture = m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, fontSize, "v"));
					auto destination_rect = RectI::centered_around(down_button_rect.center(), textTexture.texture_dimensions().x, textTexture.texture_dimensions().y);
					// TODO we may need to move the texture slightly up, check the font properties
					_down_arrow_texture = {std::move(textTexture), destination_rect};
				}
				sdl::render_texture_with_color_mod(_down_arrow_texture->textTexture.texture(), _down_arrow_texture->destinationRect);
				draw_border(down_button_rect, vertical_border_width_px(), horizontal_border_width_px());
			}
		}
	}

	draw_border(rect(), vertical_border_width_px(), horizontal_border_width_px());
}

std::vector<TextSelectionBlueprint::ReturnValue> TextSelection::selections() const {
	std::vector<TextSelectionBlueprint::ReturnValue> selections;
	for (auto& option : _options) {
		if (option.is_selected) {
			selections.emplace_back(option.blueprint_option.return_value);
		}
	}
	return selections;
}

void TextSelection::set_options(const TextSelectionBlueprint::Options& options) {
	auto copy = options;
	set_options(std::move(copy));
}
void TextSelection::set_options(TextSelectionBlueprint::Options&& options) {
	_options.clear();
	_options.resize(options.size());
	for (size_t i = 0; i < options.size(); ++i) {
		_options[i].blueprint_option = std::move(options[i]);
	}
	// Applicable to +/- selection and dropdown, select the first item
	if (text_list_selection_blueprint().line_count == 0 || text_list_selection_blueprint().line_count == 1) {
		if (not _options.empty()) {
			_options[0].is_selected = true;
			if (text_list_selection_blueprint().on_action) {
				text_list_selection_blueprint().on_action(*this);
			}
		}
	}
}
void TextSelection::set_unique_selection(int index) {
	if (0 <= index && index < I(_options.size())) {
		// Clear all selections
		for (auto& option : _options) {
			option.is_selected = false;
		}
		// Select the given option
		_options[index].is_selected = true;
		if (text_list_selection_blueprint().on_action) {
			text_list_selection_blueprint().on_action(*this);
		}
	}
}

void TextSelection::on_resize() {
	// Invalidate every font texture cache
	for (auto& option : _options) {
		option.text_texture_and_destination.reset();
	}
	_plus_texture.reset();
	_minus_texture.reset();
	_up_arrow_texture.reset();
	_down_arrow_texture.reset();
}

const TextSelectionBlueprint& TextSelection::text_list_selection_blueprint() const {
	return std::get<TextSelectionBlueprint>(blueprint->variant);
}

Action TextSelection::increment_selection() {
	if (auto current_selection = std::ranges::find_if(_options, [](auto& o) { return o.is_selected == true; });
		current_selection != _options.end()) {
		if (auto next_selection = std::next(current_selection); next_selection != _options.end()) {
			current_selection->is_selected = false;
			next_selection->is_selected = true;
			if (text_list_selection_blueprint().on_action) {
				return text_list_selection_blueprint().on_action(*this);
			}
		}
	}
	return make_continue_action();
}

Action TextSelection::decrement_selection() {
	if (auto current_selection = std::find_if(_options.rbegin(), _options.rend(), [](auto& o) { return o.is_selected == true; });
		current_selection != _options.rend()) {
		if (auto prev_selection = std::next(current_selection); prev_selection != _options.rend()) {
			current_selection->is_selected = false;
			prev_selection->is_selected = true;
			if (text_list_selection_blueprint().on_action) {
				return text_list_selection_blueprint().on_action(*this);
			}
		}
	}
	return make_continue_action();
}
