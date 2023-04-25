#include "m2/Ui.h"
#include <m2/Events.h>
#include <m2/Game.h>
#include <m2/sdl/Detail.hh>
#include <m2/Rect2i.h>
#include <SDL2/SDL_image.h>
#include <regex>
#include <filesystem>

using namespace m2;

namespace {
	SDL_Rect calculate_widget_rect(const SDL_Rect& root_rect_px, unsigned root_w, unsigned root_h, unsigned child_x, unsigned child_y, unsigned child_w, unsigned child_h) {
		auto pixels_per_unit_w = (float)root_rect_px.w / (float)root_w;
		auto pixels_per_unit_h = (float)root_rect_px.h / (float)root_h;
		return SDL_Rect{
				root_rect_px.x + (int)roundf((float)child_x * pixels_per_unit_w),
				root_rect_px.y + (int)roundf((float)child_y * pixels_per_unit_h),
				(int)roundf((float)child_w * pixels_per_unit_w),
				(int)roundf((float)child_h * pixels_per_unit_h)
		};
	}

	const Sprite* lookup_sprite(m2g::pb::SpriteType sprite_type) {
		return sprite_type ? &GAME.get_sprite(sprite_type) : nullptr;
	}

	const Sprite* lookup_initial_sprite(const ui::Blueprint::Widget *blueprint) {
		return lookup_sprite(std::get<ui::Blueprint::Widget::Image>(blueprint->variant).initial_sprite);
	}
}

ui::State::Widget::Widget(const Blueprint::Widget* blueprint) : blueprint(blueprint), rect_px({}) {}
void ui::State::Widget::update_position(const SDL_Rect &rect_px_) {
	this->rect_px = rect_px_;
}
ui::Action ui::State::Widget::handle_events(MAYBE Events &events) {
	return Action::CONTINUE;
}
ui::Action ui::State::Widget::update_content() { return Action::CONTINUE; }
void ui::State::Widget::draw() {}
void ui::State::Widget::draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align) {
	int text_w = 0, text_h = 0;
	SDL_QueryTexture(&texture, nullptr, nullptr, &text_w, &text_h);

	// Current font has 2x5 letters
	int letter_w = 2;
	int letter_h = 5;
	// Current font is rendered with 280px
	if (text_h != 280) {
		throw M2FATAL("Unexpected font height");
	}
	if ((text_w % 112) != 0) {
		throw M2FATAL("Unexpected font aspect ratio");
	}
	int char_count = text_w / 112;

	// Do the math one order above
	auto rect_w_1000 = rect.w * 1000;
	auto rect_h_1000 = rect.h * 1000;
	// Decide whether to squeeze from the sides, or top and bottom
	auto height_multiplier_1000 = rect_h_1000 / text_h;
	auto ideal_width_1000 = text_w * height_multiplier_1000;
	int provisional_text_w, provisional_text_h;
	if (ideal_width_1000 < rect_w_1000) {
		// Rect is wider than the text
		provisional_text_w = text_w * height_multiplier_1000 / 1000;
		provisional_text_h = rect.h;
	} else {
		// Rect is taller than the text
		auto width_multiplier_1000 = rect_w_1000 / text_w;
		provisional_text_w = rect.w;
		provisional_text_h = text_h * width_multiplier_1000 / 1000;
	}
	// Make sure the width is an integer multiple of character count
	provisional_text_w /= char_count;
	provisional_text_w *= char_count;
	// Apply correction based on letter aspect ratio
	int final_text_w, final_text_h;
	int letter_scale_h_1000 = 1000 * provisional_text_h / letter_h;
	int letter_scale_w_1000 = 1000 * provisional_text_w / letter_w;
	if (letter_scale_h_1000 < letter_scale_w_1000) {
		// Height is correct, apply correction to width
		int expected_letter_w = letter_w * letter_scale_h_1000 / 1000;
		final_text_w = char_count * expected_letter_w;
		final_text_h = letter_h * letter_scale_h_1000 / 1000;
	} else {
		// Width is correct, apply correction to height
		int expected_letter_h = letter_h * letter_scale_w_1000 / 1000;
		final_text_w = letter_w * letter_scale_w_1000 / 1000;
		final_text_h = expected_letter_h;
	}

	SDL_Rect dstrect{};
	dstrect.y = rect.y + rect.h / 2 - final_text_h / 2;
	dstrect.w = final_text_w;
	dstrect.h = final_text_h;
	switch (align) {
		case TextAlignment::LEFT:
			dstrect.x = rect.x;
			break;
		case TextAlignment::RIGHT:
			dstrect.x = rect.x + rect.w - final_text_w;
			break;
		default:
			dstrect.x = rect.x + rect.w / 2 - final_text_w / 2;
			break;
	}
	SDL_RenderCopy(GAME.sdlRenderer, &texture, nullptr, &dstrect);
}

ui::State::AbstractButton::AbstractButton(const ui::Blueprint::Widget *blueprint) :
		Widget(blueprint),
		kb_shortcut(
				std::visit(overloaded {
						[](const Blueprint::Widget::Text& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const Blueprint::Widget::Image& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const Blueprint::Widget::CheckboxWithText& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const auto& v) -> SDL_Scancode { (void)v; return SDL_SCANCODE_UNKNOWN; }
				}, blueprint->variant)
		),
		depressed(false) {}
ui::Action ui::State::AbstractButton::handle_events(Events &events) {
	bool run_action{};
	if (kb_shortcut != SDL_SCANCODE_UNKNOWN && SDL_IsTextInputActive() == false && events.pop_ui_key_press(kb_shortcut)) {
		run_action = true;
	} else {
		if (!depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, Rect2i{rect_px})) {
			depressed = true;
		} else if (depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, Rect2i{rect_px})) {
			run_action = true;
		}
	}
	if (run_action) {
		depressed = false;
		return std::visit(overloaded {
				[](const Blueprint::Widget::Text& v) { return v.action_callback ? v.action_callback() : Action::CONTINUE; },
				[](const Blueprint::Widget::Image& v) { return v.action_callback ? v.action_callback() : Action::CONTINUE; },
				[&](const Blueprint::Widget::CheckboxWithText& v) {
					// Overloading handle_events for CheckboxWithText is too much work, do it here
					auto checkbox_with_text_state = dynamic_cast<State::CheckboxWithText*>(this);
					checkbox_with_text_state->state = !checkbox_with_text_state->state;
					return v.action_callback ? v.action_callback(checkbox_with_text_state->state) : Action::CONTINUE;
				},
				[](MAYBE const auto& v) { return Action::CONTINUE; }
		}, blueprint->variant);
	} else {
		return Action::CONTINUE;
	}
}

ui::State::Image::Image(const ui::Blueprint::Widget* blueprint) : AbstractButton(blueprint), sprite(lookup_initial_sprite(blueprint)) {}
ui::Action ui::State::Image::update_content() {
	auto& image_blueprint = std::get<Blueprint::Widget::Image>(blueprint->variant);
	if (image_blueprint.update_callback) {
		auto[action, opt_sprite] = image_blueprint.update_callback();
		if (action == Action::CONTINUE && opt_sprite) {
			sprite =  lookup_sprite(*opt_sprite);
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}
void ui::State::Image::draw() {
	State::draw_background_color(rect_px, blueprint->background_color);
	if (sprite) {
		// Make sure sprite is drawn square
		SDL_Rect dstrect;
		if (rect_px.h < rect_px.w) {
			dstrect = {
					.x = rect_px.x + (rect_px.w - rect_px.h) / 2,
					.y = rect_px.y,
					.w = rect_px.h,
					.h = rect_px.h
			};
		} else {
			dstrect = {
					.x = rect_px.x,
					.y = rect_px.y + (rect_px.h - rect_px.w) / 2,
					.w = rect_px.w,
					.h = rect_px.w
			};
		}
		auto srcrect = sdl::to_rect(sprite->sprite().rect());
		SDL_RenderCopy(GAME.sdlRenderer, sprite->sprite_sheet().texture(), &srcrect, &dstrect);
	}
	State::draw_border(rect_px, blueprint->border_width_px);
}

ui::State::Text::Text(const Blueprint::Widget* blueprint) : AbstractButton(blueprint), font_texture(sdl::generate_font(std::get<Blueprint::Widget::Text>(blueprint->variant).initial_text.data())) {}
ui::Action ui::State::Text::update_content() {
	auto& text_blueprint = std::get<Blueprint::Widget::Text>(blueprint->variant);
	if (text_blueprint.update_callback) {
		auto[action, optional_string] = text_blueprint.update_callback();
		if (action == Action::CONTINUE && optional_string) {
			font_texture = sdl::generate_font(optional_string->c_str());
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}
void ui::State::Text::draw() {
	State::draw_background_color(rect_px, blueprint->background_color);
	if (font_texture) {
		draw_text(sdl::expand_rect(rect_px, -static_cast<int>(blueprint->padding_width_px)), *font_texture, std::get<Blueprint::Widget::Text>(blueprint->variant).alignment);
	}
	State::draw_border(rect_px, blueprint->border_width_px);
}

ui::State::NestedUi::NestedUi(const Blueprint::Widget *blueprint) : Widget(blueprint) {
	ui = std::make_unique<State>(std::get<Blueprint::Widget::NestedUi>(blueprint->variant).ui);
}
void ui::State::NestedUi::update_position(const SDL_Rect &rect_px_) {
	this->rect_px = rect_px_;
	ui->update_positions(rect_px_);
}
ui::Action ui::State::NestedUi::handle_events(Events &events) {
	return ui->handle_events(events);
}
ui::Action ui::State::NestedUi::update_content() {
	return ui->update_contents();
}
void ui::State::NestedUi::draw() {
	ui->draw();
}

ui::State::ProgressBar::ProgressBar(const Blueprint::Widget* blueprint) : Widget(blueprint), progress(std::get<Blueprint::Widget::ProgressBar>(blueprint->variant).initial_progress) {}
ui::Action ui::State::ProgressBar::update_content() {
	auto& pb_blueprint = std::get<Blueprint::Widget::ProgressBar>(blueprint->variant);
	if (pb_blueprint.update_callback) {
		progress = pb_blueprint.update_callback();
	}
	return Action::CONTINUE;
}
void ui::State::ProgressBar::draw() {
	auto& pb_blueprint = std::get<Blueprint::Widget::ProgressBar>(blueprint->variant);
	// Background
	State::draw_background_color(rect_px, blueprint->background_color);
	// Bar
	auto filled_dstrect = SDL_Rect{
			rect_px.x,
			rect_px.y,
			(int)roundf((float)rect_px.w * progress),
			rect_px.h
	};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, pb_blueprint.bar_color.r, pb_blueprint.bar_color.g, pb_blueprint.bar_color.b, pb_blueprint.bar_color.a);
	SDL_RenderFillRect(GAME.sdlRenderer, &filled_dstrect);
	// Foreground
	State::draw_border(rect_px, blueprint->border_width_px);
}

ui::State::TextInput::TextInput(const Blueprint::Widget* blueprint) : Widget(blueprint), font_texture(nullptr) {
	text_input << std::get<Blueprint::Widget::TextInput>(blueprint->variant).initial_text;
	SDL_StartTextInput();
}
ui::State::TextInput::~TextInput() {
	SDL_StopTextInput();
}
ui::Action ui::State::TextInput::handle_events(Events& events) {
	if (events.pop_key_press(Key::MENU)) {
		return Action::RETURN;
	} else if (events.pop_key_press(Key::ENTER)) {
		return std::get<Blueprint::Widget::TextInput>(blueprint->variant).action_callback(text_input);
	} else if (events.pop_key_press(Key::BACKSPACE)) {
		auto text_input_str = text_input.str();
		if (not text_input_str.empty()) {
			text_input = std::stringstream{text_input_str.substr(0, text_input_str.length() - 1)};
			text_input.seekp(0, std::ios::end);
		}
	} else {
		auto opt_text_input = events.pop_text_input();
		if (opt_text_input) {
			text_input << *opt_text_input;
		}
	}
	return Action::CONTINUE;
}
ui::Action ui::State::TextInput::update_content() {
	auto new_str = text_input.str() + '_';
	if (new_str != font_texture_str) {
		font_texture = sdl::generate_font(new_str.c_str());
		font_texture_str = new_str;
	}
	return Action::CONTINUE;
}
void ui::State::TextInput::draw() {
	State::draw_background_color(rect_px, blueprint->background_color);
	if (font_texture) {
		draw_text(rect_px, *font_texture, TextAlignment::LEFT);
	}
	State::draw_border(rect_px, blueprint->border_width_px);
}

ui::State::ImageSelection::ImageSelection(const Blueprint::Widget* blueprint) : Widget(blueprint), selection(std::get<Blueprint::Widget::ImageSelection>(blueprint->variant).initial_selection) {}
ui::Action ui::State::ImageSelection::handle_events(Events& events) {
	auto rect = Rect2i{rect_px};
	auto buttons_rect = rect.trim_top(rect.w);
	auto inc_button_rect = buttons_rect.trim_left(buttons_rect.w / 2);
	auto dec_button_rect = buttons_rect.trim_right(buttons_rect.w / 2);

	const auto& image_selection = std::get<Blueprint::Widget::ImageSelection>(blueprint->variant);

	bool selection_changed = false;
	if (!inc_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = true;
		dec_depressed = false;
	} else if (!dec_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = true;
		inc_depressed = false;
	} else if (inc_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = false;
		if (selection + 1 < image_selection.list.size()) {
			++selection;
			selection_changed = true;
		}
	} else if (dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = false;
		if (0 < selection) {
			--selection;
			selection_changed = true;
		}
	}

	if (selection_changed) {
		const auto& action_callback = image_selection.action_callback;
		if (action_callback) {
			return action_callback(image_selection.list[selection]);
		}
	}

	return Action::CONTINUE;
}
void ui::State::ImageSelection::draw() {
	auto rect = Rect2i{rect_px};
	auto image_rect = rect.trim_bottom(rect.h - rect.w);
	auto buttons_rect = rect.trim_top(rect.w);
	auto inc_button_rect = buttons_rect.trim_left(buttons_rect.w / 2);
	auto inc_button_symbol_rect = inc_button_rect.trim_to_square().trim(5);
	auto dec_button_rect = buttons_rect.trim_right(buttons_rect.w / 2);
	auto dec_button_symbol_rect = dec_button_rect.trim_to_square().trim(5);

	draw_background_color(rect_px, blueprint->background_color);

	const auto& image_selection = std::get<Blueprint::Widget::ImageSelection>(blueprint->variant);
	const auto& sprite = GAME.get_sprite(image_selection.list[selection]);
	auto sprite_srcrect = sdl::to_rect(sprite.sprite().rect());
	auto sprite_dstrect = (SDL_Rect)image_rect;
	SDL_RenderCopy(GAME.sdlRenderer, sprite.sprite_sheet().texture(), &sprite_srcrect, &sprite_dstrect);

	static SDL_Texture* up_symbol = IMG_LoadTexture(GAME.sdlRenderer, "resource/up-symbol.svg");
	auto up_dstrect = (SDL_Rect)inc_button_symbol_rect;
	SDL_RenderCopy(GAME.sdlRenderer, up_symbol, nullptr, &up_dstrect);
	draw_border((SDL_Rect)inc_button_rect, blueprint->border_width_px);

	static SDL_Texture* down_symbol = IMG_LoadTexture(GAME.sdlRenderer, "resource/down-symbol.svg");
	auto down_dstrect = (SDL_Rect)dec_button_symbol_rect;
	SDL_RenderCopy(GAME.sdlRenderer, down_symbol, nullptr, &down_dstrect);
	draw_border((SDL_Rect)dec_button_rect, blueprint->border_width_px);

	draw_border(rect_px, blueprint->border_width_px);
}

ui::State::TextSelection::TextSelection(const Blueprint::Widget* blueprint) : Widget(blueprint), selection(std::get<Blueprint::Widget::TextSelection>(blueprint->variant).initial_selection), font_texture(sdl::generate_font(std::get<Blueprint::Widget::TextSelection>(blueprint->variant).list[selection].c_str())) {}
ui::Action ui::State::TextSelection::handle_events(Events& events) {
	auto rect = Rect2i{rect_px};
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);

	const auto& text_selection = std::get<Blueprint::Widget::TextSelection>(blueprint->variant);

	bool selection_changed = false;
	if (!inc_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = true;
		dec_depressed = false;
	} else if (!dec_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = true;
		inc_depressed = false;
	} else if (inc_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = false;
		if (selection + 1 < text_selection.list.size()) {
			++selection;
			selection_changed = true;
		}
	} else if (dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = false;
		if (0 < selection) {
			--selection;
			selection_changed = true;
		}
	}

	if (selection_changed) {
		font_texture = sdl::generate_font(text_selection.list[selection].c_str());

		const auto& action_callback = text_selection.action_callback;
		if (action_callback) {
			return action_callback(text_selection.list[selection]);
		}
	}

	return Action::CONTINUE;
}
void ui::State::TextSelection::draw() {
	auto rect = Rect2i{rect_px};
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

	static SDL_Texture* up_symbol = IMG_LoadTexture(GAME.sdlRenderer, "resource/up-symbol.svg");
	auto up_dstrect = (SDL_Rect)inc_button_symbol_rect;
	SDL_RenderCopy(GAME.sdlRenderer, up_symbol, nullptr, &up_dstrect);
	draw_border((SDL_Rect)inc_button_rect, blueprint->border_width_px);

	static SDL_Texture* down_symbol = IMG_LoadTexture(GAME.sdlRenderer, "resource/down-symbol.svg");
	auto down_dstrect = (SDL_Rect)dec_button_symbol_rect;
	SDL_RenderCopy(GAME.sdlRenderer, down_symbol, nullptr, &down_dstrect);
	draw_border((SDL_Rect)dec_button_rect, blueprint->border_width_px);

	draw_border(rect_px, blueprint->border_width_px);
}

ui::State::IntegerSelection::IntegerSelection(const Blueprint::Widget *blueprint) : Widget(blueprint), value(std::get<Blueprint::Widget::IntegerSelection>(blueprint->variant).initial_value), font_texture(sdl::generate_font(std::to_string(value).c_str())) {}
ui::Action ui::State::IntegerSelection::handle_events(Events& events) {
	auto rect = Rect2i{rect_px};
	auto buttons_rect = rect.trim_left(rect.w - rect.h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);

	const auto& integer_selection = std::get<Blueprint::Widget::IntegerSelection>(blueprint->variant);

	bool selection_changed = false;
	if (!inc_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = true;
		dec_depressed = false;
	} else if (!dec_depressed && events.pop_mouse_button_press(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = true;
		inc_depressed = false;
	} else if (inc_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, inc_button_rect)) {
		inc_depressed = false;
		if (value < integer_selection.max_value) {
			++value;
			selection_changed = true;
		}
	} else if (dec_depressed && events.pop_mouse_button_release(MouseButton::PRIMARY, dec_button_rect)) {
		dec_depressed = false;
		if (integer_selection.min_value < value) {
			--value;
			selection_changed = true;
		}
	}

	if (selection_changed) {
		font_texture = sdl::generate_font(std::to_string(value).c_str());

		const auto& action_callback = integer_selection.action_callback;
		if (action_callback) {
			return action_callback(value);
		}
	}

	return Action::CONTINUE;
}
ui::Action ui::State::IntegerSelection::update_content() {
	auto& pb_blueprint = std::get<Blueprint::Widget::IntegerSelection>(blueprint->variant);
	if (pb_blueprint.update_callback) {
		auto optional_value = pb_blueprint.update_callback();
		if (optional_value) {
			value = *optional_value;
			font_texture = sdl::generate_font(std::to_string(value).c_str());
		}
	}
	return Action::CONTINUE;
}
void ui::State::IntegerSelection::draw() {
	auto rect = Rect2i{rect_px};
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

	static SDL_Texture* up_symbol = IMG_LoadTexture(GAME.sdlRenderer, "resource/up-symbol.svg");
	auto up_dstrect = (SDL_Rect)inc_button_symbol_rect;
	SDL_RenderCopy(GAME.sdlRenderer, up_symbol, nullptr, &up_dstrect);
	draw_border((SDL_Rect)inc_button_rect, blueprint->border_width_px);

	static SDL_Texture* down_symbol = IMG_LoadTexture(GAME.sdlRenderer, "resource/down-symbol.svg");
	auto down_dstrect = (SDL_Rect)dec_button_symbol_rect;
	SDL_RenderCopy(GAME.sdlRenderer, down_symbol, nullptr, &down_dstrect);
	draw_border((SDL_Rect)dec_button_rect, blueprint->border_width_px);

	draw_border(rect_px, blueprint->border_width_px);
}

ui::State::CheckboxWithText::CheckboxWithText(const Blueprint::Widget *blueprint) : AbstractButton(blueprint), state(std::get<Blueprint::Widget::CheckboxWithText>(blueprint->variant).initial_state), font_texture(sdl::generate_font(std::get<Blueprint::Widget::CheckboxWithText>(blueprint->variant).text.c_str())) {}
void ui::State::CheckboxWithText::draw() {
	// Background
	State::draw_background_color(rect_px, blueprint->background_color);
	// Checkbox
	auto filled_dstrect = SDL_Rect{rect_px.x, rect_px.y, rect_px.h, rect_px.h};
	SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 255, 255, 255);
	SDL_RenderFillRect(GAME.sdlRenderer, &filled_dstrect);
	if (!state) {
		auto empty_dstrect = SDL_Rect{rect_px.x + 1, rect_px.y + 1, rect_px.h - 2, rect_px.h - 2};
		SDL_SetRenderDrawColor(GAME.sdlRenderer, blueprint->background_color.r, blueprint->background_color.g, blueprint->background_color.b, blueprint->background_color.a);
		SDL_RenderFillRect(GAME.sdlRenderer, &empty_dstrect);
	}
	// Text
	if (font_texture) {
		auto text_rect = Rect2i{rect_px};
		draw_text((SDL_Rect)text_rect.trim_left(rect_px.h).trim((int)blueprint->padding_width_px), *font_texture, TextAlignment::LEFT);
	}
	// Border
	State::draw_border(rect_px, blueprint->border_width_px);
}

void ui::State::draw_background_color(const SDL_Rect& rect, const SDL_Color& color) {
    if (!color.r && !color.g && !color.b && !color.a) {
		// If the color is all zeros, user probably didn't initialize the Color at all
		// Paint background to default background color
        SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, color.r, color.g, color.b, color.a);
    }
    SDL_RenderFillRect(GAME.sdlRenderer, &rect);
}
void ui::State::draw_border(const SDL_Rect& rect, unsigned border_width_px) {
    if (border_width_px) {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(GAME.sdlRenderer, &rect);
    }
}
ui::State::State() : blueprint(nullptr), rect_px() {}
ui::State::State(const Blueprint* blueprint) : blueprint(blueprint), rect_px({}) {
    for (const auto& widget_blueprint : blueprint->widgets) {
        widgets.push_back(create_widget_state(widget_blueprint));
    }
}
void ui::State::update_positions(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
    for (auto& widget_state : widgets) {
        SDL_Rect widget_rect = calculate_widget_rect(rect_px_, blueprint->w, blueprint->h, widget_state->blueprint->x, widget_state->blueprint->y, widget_state->blueprint->w, widget_state->blueprint->h);
        widget_state->update_position(widget_rect);
    }
}
ui::Action ui::State::handle_events(Events& events) {
    Action return_value = Action::CONTINUE;
    for (auto& widget : widgets) {
        if ((return_value = widget->handle_events(events)) != Action::CONTINUE) {
            break;
        }
    }
    return return_value;
}
ui::Action ui::State::update_contents() {
    Action return_value = Action::CONTINUE;
    for (auto& widget : widgets) {
        if ((return_value = widget->update_content()) != Action::CONTINUE) {
            break;
        }
    }
    return return_value;
}
void ui::State::draw() {
    draw_background_color(rect_px, blueprint->background_color);
    for (auto& widget : widgets) {
        widget->draw();
    }
    draw_border(rect_px, blueprint->border_width_px);
}

std::unique_ptr<ui::State::Widget> ui::State::create_widget_state(const Blueprint::Widget& blueprint) {
	std::unique_ptr<State::Widget> state;
	if (std::holds_alternative<Blueprint::Widget::NestedUi>(blueprint.variant)) {
		state = std::make_unique<State::NestedUi>(&blueprint);
	} else if (std::holds_alternative<Blueprint::Widget::Text>(blueprint.variant)) {
		state = std::make_unique<State::Text>(&blueprint);
	} else if (std::holds_alternative<Blueprint::Widget::TextInput>(blueprint.variant)) {
		state = std::make_unique<State::TextInput>(&blueprint);
	} else if (std::holds_alternative<Blueprint::Widget::Image>(blueprint.variant)) {
		state = std::make_unique<State::Image>(&blueprint);
	} else if (std::holds_alternative<Blueprint::Widget::ProgressBar>(blueprint.variant)) {
		state = std::make_unique<State::ProgressBar>(&blueprint);
	} else if (std::holds_alternative<Blueprint::Widget::ImageSelection>(blueprint.variant)) {
		state = std::make_unique<State::ImageSelection>(&blueprint);
	} else if (std::holds_alternative<Blueprint::Widget::TextSelection>(blueprint.variant)) {
		state = std::make_unique<State::TextSelection>(&blueprint);
	} else if (std::holds_alternative<Blueprint::Widget::IntegerSelection>(blueprint.variant)) {
		state = std::make_unique<State::IntegerSelection>(&blueprint);
	} else if (std::holds_alternative<Blueprint::Widget::CheckboxWithText>(blueprint.variant)) {
		state = std::make_unique<State::CheckboxWithText>(&blueprint);
	} else {
		throw M2FATAL("Implementation");
	}
	return state;
}

ui::Action ui::execute_blocking(const Blueprint *blueprint) {
	return execute_blocking(blueprint, GAME.windowRect);
}
ui::Action ui::execute_blocking(const Blueprint *blueprint, SDL_Rect rect) {
	auto execute_start_ticks = sdl::get_ticks();

	// Save relation to window, use in case of resize
	const SDL_Rect& winrect = GAME.windowRect;
	auto relation_to_window = SDL_FRect{
		(float)(rect.x - winrect.x) / (float)winrect.w,
		(float)(rect.y - winrect.y) / (float)winrect.h,
		(float)rect.w / (float)winrect.w,
		(float)rect.h / (float)winrect.h,
	};

	// Get screenshot
	int w, h;
	SDL_GetRendererOutputSize(GAME.sdlRenderer, &w, &h);
	auto* surface = SDL_CreateRGBSurface(0, w, h, 24, 0xFF, 0xFF00, 0xFF0000, 0);
	SDL_RenderReadPixels(GAME.sdlRenderer, nullptr, SDL_PIXELFORMAT_RGB24, surface->pixels, surface->pitch);
	std::unique_ptr<SDL_Texture, sdl::TextureDeleter> texture(SDL_CreateTextureFromSurface(GAME.sdlRenderer, surface));
	SDL_FreeSurface(surface);

	Action return_value;

    State state(blueprint);
    state.update_positions(rect);
	if ((return_value = state.update_contents()) != Action::CONTINUE) {
		GAME.add_pause_ticks(sdl::get_ticks_since(execute_start_ticks));
		return return_value;
	}

    Events events;
    while (true) {
        ////////////////////////////////////////////////////////////////////////
        //////////////////////////// EVENT HANDLING ////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        events.clear();
        if (events.gather()) {
            if (events.pop_quit()) {
				GAME.add_pause_ticks(sdl::get_ticks_since(execute_start_ticks));
                return Action::QUIT;
            }
			auto window_resize = events.pop_window_resize();
            if (window_resize) {
				GAME.update_window_dims(window_resize->x, window_resize->y);
                state.update_positions(SDL_Rect{
	                (int)round((float)winrect.x + relation_to_window.x * (float)winrect.w),
	                (int)round((float)winrect.y + relation_to_window.y * (float)winrect.h),
	                (int)round(relation_to_window.w * (float)winrect.w),
	                (int)round(relation_to_window.h * (float)winrect.h)
                });
            }
            if ((return_value = state.handle_events(events)) != Action::CONTINUE) {
				// TODO if execute_blocking is executed recursively, pause_ticks calculation becomes incorrect
				GAME.add_pause_ticks(sdl::get_ticks_since(execute_start_ticks));
                return return_value;
            }
        }
        //////////////////////// END OF EVENT HANDLING /////////////////////////
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        /////////////////////////////// GRAPHICS ///////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        // Draw ui
		if ((return_value = state.update_contents()) != Action::CONTINUE) {
			GAME.add_pause_ticks(sdl::get_ticks_since(execute_start_ticks));
			return return_value;
		}
	    // Clear screen
	    SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
	    SDL_RenderClear(GAME.sdlRenderer);
	    SDL_RenderCopy(GAME.sdlRenderer, texture.get(), nullptr, nullptr);
        state.draw();
        // Present
        SDL_RenderPresent(GAME.sdlRenderer);
        /////////////////////////// END OF GRAPHICS ////////////////////////////
        ////////////////////////////////////////////////////////////////////////
    }
}

const ui::Blueprint::Widget::Variant command_input_variant = ui::Blueprint::Widget::TextInput{
        .initial_text = "",
        .action_callback = [](std::stringstream& ss) -> ui::Action {
            auto command = ss.str();
	        ss = std::stringstream();
	        GAME.console_output.emplace_back(">> " + command);

            if (std::regex_match(command, std::regex{"ledit(\\s.*)?"})) {
				std::smatch match_results;
				if (std::regex_match(command, match_results, std::regex{"ledit\\s+(.+)"})) {
					auto load_result = GAME.load_level_editor(match_results.str(1));
					if (load_result) {
						return ui::Action::RETURN;
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("ledit usage:");
					GAME.console_output.emplace_back(".. file_name - open level editor with file");
				}
				return ui::Action::CONTINUE;
			} else if (std::regex_match(command, std::regex{"pedit(\\s.*)?"})) {
				std::smatch match_results;
				if (std::regex_match(command, match_results, std::regex{"pedit\\s+([0-9]+)\\s+([0-9]+)\\s+(.+)"})) {
					auto x_offset = strtol(match_results.str(1).c_str(), nullptr, 0);
					auto y_offset = strtol(match_results.str(2).c_str(), nullptr, 0);
					auto load_result = GAME.load_pixel_editor(match_results.str(3), (int) x_offset, (int) y_offset);
					if (load_result) {
						return ui::Action::RETURN;
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("pedit usage:");
					GAME.console_output.emplace_back(".. x_offset y_offset file_name - open pixel editor with file");
				}
				return ui::Action::CONTINUE;
			} else if (command == "quit") {
                return ui::Action::QUIT;
            } else if (command.empty()) {
				// Do nothing
			} else {
	            GAME.console_output.emplace_back("Available commands:");
	            GAME.console_output.emplace_back("help - display this help");
	            GAME.console_output.emplace_back("ledit - open level editor");
	            GAME.console_output.emplace_back("pedit - open pixel editor");
	            GAME.console_output.emplace_back("quit - quit game");
            }

	        return ui::Action::CONTINUE;
        }
};
template <unsigned INDEX>
ui::Blueprint::Widget::Variant command_output_variant() {
    return ui::Blueprint::Widget::Text{
        .initial_text = "",
        .alignment = ui::TextAlignment::LEFT,
        .update_callback = []() -> std::pair<ui::Action,std::optional<std::string>> {
            return {ui::Action::CONTINUE, INDEX < GAME.console_output.size() ? GAME.console_output[GAME.console_output.size() - INDEX - 1] : std::string()};
        }
    };
}
const ui::Blueprint ui::console_ui = {
        .w = 1, .h = 25,
        .widgets = {
			Blueprint::Widget{.x = 0, .y = 0, .w = 1, .h = 1, .variant = command_output_variant<23>()},
			Blueprint::Widget{.x = 0, .y = 1, .w = 1, .h = 1, .variant = command_output_variant<22>()},
			Blueprint::Widget{.x = 0, .y = 2, .w = 1, .h = 1, .variant = command_output_variant<21>()},
			Blueprint::Widget{.x = 0, .y = 3, .w = 1, .h = 1, .variant = command_output_variant<20>()},
			Blueprint::Widget{.x = 0, .y = 4, .w = 1, .h = 1, .variant = command_output_variant<19>()},
			Blueprint::Widget{.x = 0, .y = 5, .w = 1, .h = 1, .variant = command_output_variant<18>()},
			Blueprint::Widget{.x = 0, .y = 6, .w = 1, .h = 1, .variant = command_output_variant<17>()},
			Blueprint::Widget{.x = 0, .y = 7, .w = 1, .h = 1, .variant = command_output_variant<16>()},
			Blueprint::Widget{.x = 0, .y = 8, .w = 1, .h = 1, .variant = command_output_variant<15>()},
			Blueprint::Widget{.x = 0, .y = 9, .w = 1, .h = 1, .variant = command_output_variant<14>()},
			Blueprint::Widget{.x = 0, .y = 10, .w = 1, .h = 1, .variant = command_output_variant<13>()},
			Blueprint::Widget{.x = 0, .y = 11, .w = 1, .h = 1, .variant = command_output_variant<12>()},
			Blueprint::Widget{.x = 0, .y = 12, .w = 1, .h = 1, .variant = command_output_variant<11>()},
			Blueprint::Widget{.x = 0, .y = 13, .w = 1, .h = 1, .variant = command_output_variant<10>()},
			Blueprint::Widget{.x = 0, .y = 14, .w = 1, .h = 1, .variant = command_output_variant<9>()},
			Blueprint::Widget{.x = 0, .y = 15, .w = 1, .h = 1, .variant = command_output_variant<8>()},
			Blueprint::Widget{.x = 0, .y = 16, .w = 1, .h = 1, .variant = command_output_variant<7>()},
			Blueprint::Widget{.x = 0, .y = 17, .w = 1, .h = 1, .variant = command_output_variant<6>()},
			Blueprint::Widget{.x = 0, .y = 18, .w = 1, .h = 1, .variant = command_output_variant<5>()},
			Blueprint::Widget{.x = 0, .y = 19, .w = 1, .h = 1, .variant = command_output_variant<4>()},
			Blueprint::Widget{.x = 0, .y = 20, .w = 1, .h = 1, .variant = command_output_variant<3>()},
			Blueprint::Widget{.x = 0, .y = 21, .w = 1, .h = 1, .variant = command_output_variant<2>()},
			Blueprint::Widget{.x = 0, .y = 22, .w = 1, .h = 1, .variant = command_output_variant<1>()},
			Blueprint::Widget{.x = 0, .y = 23, .w = 1, .h = 1, .variant = command_output_variant<0>()},
			Blueprint::Widget{
		        .x = 0, .y = 24, .w = 1, .h = 1,
				.background_color = SDL_Color{27, 27, 27, 255},
		        .variant = command_input_variant
	        }
        }
};
