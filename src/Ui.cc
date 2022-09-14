#include "m2/Ui.h"
#include <m2/Events.h>
#include <m2/Game.hh>
#include <m2/SdlUtils.hh>
#include <regex>
#include <filesystem>

using namespace m2;
using namespace m2::ui;

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

	const Sprite* lookup_sprite(const std::string& key) {
		return &GAME.sprite_key_to_sprite_map.at(key);
	}

	const Sprite* lookup_initial_sprite(const Blueprint::Widget *blueprint) {
		const auto& initial_sprite_key = std::get<Blueprint::Widget::Image>(blueprint->variant).initial_sprite_key;
		if (initial_sprite_key.empty()) {
			return nullptr;
		} else {
			return lookup_sprite(initial_sprite_key);
		}
	}
}

State::Widget::Widget(const Blueprint::Widget* blueprint) : blueprint(blueprint), rect_px({}) {}
void State::Widget::update_position(const SDL_Rect &rect_px_) {
	this->rect_px = rect_px_;
}
Action State::Widget::handle_events(Events &events) {
	(void)events;
	return Action::CONTINUE;
}
Action State::Widget::update_content() { return Action::CONTINUE; }
void State::Widget::draw() {}
SDL_Texture* State::Widget::generate_font_texture(const char* text) {
	SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME.ttfFont, text, SDL_Color{255, 255, 255, 255});
	SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME.sdlRenderer, textSurf);
	SDL_FreeSurface(textSurf);
	return texture;
}
void State::Widget::draw_text(const SDL_Rect& rect, SDL_Texture& texture, TextAlignment align) {
	int text_w = 0, text_h = 0;
	SDL_QueryTexture(&texture, nullptr, nullptr, &text_w, &text_h);

	SDL_Rect dstrect;
	dstrect.y = rect.h < text_h ? rect.y : rect.y + rect.h / 2 - text_h / 2;
	dstrect.w = rect.w < text_w ? rect.w : text_w;
	dstrect.h = rect.h < text_h ? rect.h : text_h;
	switch (align) {
		case TextAlignment::LEFT:
			dstrect.x = rect.x;
			break;
		case TextAlignment::RIGHT:
			dstrect.x = rect.w < text_w ? rect.x : rect.x + rect.w - text_w;
			break;
		default:
			dstrect.x = rect.w < text_w ? rect.x : rect.x + rect.w / 2 - text_w / 2;
			break;
	}
	SDL_RenderCopy(GAME.sdlRenderer, &texture, nullptr, &dstrect);
}

State::AbstractButton::AbstractButton(const Blueprint::Widget *blueprint) :
		Widget(blueprint),
		kb_shortcut(
				std::visit(overloaded {
						[](const Blueprint::Widget::Text& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const Blueprint::Widget::Image& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const auto& v) -> SDL_Scancode { (void)v; return SDL_SCANCODE_UNKNOWN; }
				}, blueprint->variant)
		),
		depressed(false) {}
Action State::AbstractButton::handle_events(Events &events) {
	bool run_action = false;
	if (kb_shortcut != SDL_SCANCODE_UNKNOWN && SDL_IsTextInputActive() == false && events.pop_ui_key_press(kb_shortcut)) {
		run_action = true;
	} else {
		auto mouse_position = events.mouse_position();
		SDL_Point sdl_mouse_position = {mouse_position.x, mouse_position.y};
		if (SDL_PointInRect(&sdl_mouse_position, &rect_px)) {
			if (not depressed && events.pop_mouse_button_press(MouseButton::PRIMARY)) {
				depressed = true;
			} else if (depressed && events.pop_mouse_button_release(MouseButton::PRIMARY)) {
				run_action = true;
			}
		}
	}
	if (run_action) {
		depressed = false;
		return std::visit(overloaded {
				[](const Blueprint::Widget::Text& v) { return v.action_callback ? v.action_callback() : Action::CONTINUE; },
				[](const Blueprint::Widget::Image& v) { return v.action_callback ? v.action_callback() : Action::CONTINUE; },
				[](const auto& v) { (void)v; return Action::CONTINUE; }
		}, blueprint->variant);
	} else {
		return Action::CONTINUE;
	}
}

State::Image::Image(const Blueprint::Widget* blueprint) : AbstractButton(blueprint), sprite(lookup_initial_sprite(blueprint)) {}
Action State::Image::update_content() {
	auto& image_blueprint = std::get<Blueprint::Widget::Image>(blueprint->variant);
	if (image_blueprint.update_callback) {
		auto[action, opt_key] = image_blueprint.update_callback();
		if (action == Action::CONTINUE && opt_key) {
			sprite = lookup_sprite(*opt_key);
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}
void State::Image::draw() {
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

State::Text::Text(const Blueprint::Widget* blueprint) : AbstractButton(blueprint), font_texture(generate_font_texture(std::get<Blueprint::Widget::Text>(blueprint->variant).initial_text.data())) {}
State::Text::~Text() {
	SDL_DestroyTexture(font_texture);
}
Action State::Text::update_content() {
	auto& text_blueprint = std::get<Blueprint::Widget::Text>(blueprint->variant);
	if (text_blueprint.update_callback) {
		auto[action, optional_string] = text_blueprint.update_callback();
		if (action == Action::CONTINUE && optional_string) {
			if (font_texture) {
				SDL_DestroyTexture(font_texture);
			}
			font_texture = generate_font_texture(optional_string->c_str());
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}
void State::Text::draw() {
	State::draw_background_color(rect_px, blueprint->background_color);
	draw_text(sdl::expand_rect(rect_px, -static_cast<int>(blueprint->padding_width_px)), *font_texture, std::get<Blueprint::Widget::Text>(blueprint->variant).alignment);
	State::draw_border(rect_px, blueprint->border_width_px);
}

State::NestedUi::NestedUi(const Blueprint::Widget *blueprint) : Widget(blueprint) {
	ui = std::make_unique<State>(std::get<Blueprint::Widget::NestedUi>(blueprint->variant).ui);
}
void State::NestedUi::update_position(const SDL_Rect &rect_px_) {
	this->rect_px = rect_px_;
	ui->update_positions(rect_px_);
}
Action State::NestedUi::handle_events(Events &events) {
	return ui->handle_events(events);
}
Action State::NestedUi::update_content() {
	return ui->update_contents();
}
void State::NestedUi::draw() {
	ui->draw();
}

State::ProgressBar::ProgressBar(const Blueprint::Widget* blueprint) : Widget(blueprint), progress(std::get<Blueprint::Widget::ProgressBar>(blueprint->variant).initial_progress) {}
m2::ui::Action State::ProgressBar::update_content() {
	auto& pb_blueprint = std::get<Blueprint::Widget::ProgressBar>(blueprint->variant);
	if (pb_blueprint.update_callback) {
		progress = pb_blueprint.update_callback();
	}
	return Action::CONTINUE;
}
void State::ProgressBar::draw() {
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
	SDL_SetRenderDrawColor(GAME.sdlRenderer, pb_blueprint.bar_color.r, pb_blueprint.bar_color.g,
						   pb_blueprint.bar_color.b, pb_blueprint.bar_color.a);
	SDL_RenderFillRect(GAME.sdlRenderer, &filled_dstrect);
	// Foreground
	State::draw_border(rect_px, blueprint->border_width_px);
}

State::TextInput::TextInput(const Blueprint::Widget* blueprint) : Widget(blueprint), font_texture(nullptr) {
	text_input << std::get<Blueprint::Widget::TextInput>(blueprint->variant).initial_text;
	SDL_StartTextInput();
}
State::TextInput::~TextInput() {
	if (font_texture) {
		SDL_DestroyTexture(font_texture);
	}
	SDL_StopTextInput();
}
Action State::TextInput::handle_events(Events& events) {
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
Action State::TextInput::update_content() {
	auto new_str = text_input.str();
	if (new_str != font_texture_str) {
		if (font_texture) {
			SDL_DestroyTexture(font_texture);
		}
		font_texture = generate_font_texture(new_str.c_str());
		font_texture_str = new_str;
	}
	return Action::CONTINUE;
}
void State::TextInput::draw() {
	State::draw_background_color(rect_px, blueprint->background_color);
	draw_text(rect_px, *font_texture, TextAlignment::LEFT);
	State::draw_border(rect_px, blueprint->border_width_px);
}

void State::draw_background_color(const SDL_Rect& rect, const SDL_Color& color) {
    if (color.a == 0) {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, color.r, color.g, color.b, color.a);
    }
    SDL_RenderFillRect(GAME.sdlRenderer, &rect);
}
void State::draw_border(const SDL_Rect& rect, unsigned border_width_px) {
    if (border_width_px) {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(GAME.sdlRenderer, &rect);
    }
}
State::State() : blueprint(nullptr), rect_px() {}
State::State(const Blueprint* blueprint) : blueprint(blueprint), rect_px({}) {
    for (const auto& widget_blueprint : blueprint->widgets) {
        widgets.push_back(create_widget_state(widget_blueprint));
    }
}
void State::update_positions(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
    for (auto& widget_state : widgets) {
        SDL_Rect widget_rect = calculate_widget_rect(rect_px_, blueprint->w, blueprint->h, widget_state->blueprint->x, widget_state->blueprint->y, widget_state->blueprint->w, widget_state->blueprint->h);
        widget_state->update_position(widget_rect);
    }
}
Action State::handle_events(Events& events) {
    Action return_value = Action::CONTINUE;
    for (auto& widget : widgets) {
        if ((return_value = widget->handle_events(events)) != Action::CONTINUE) {
            break;
        }
    }
    return return_value;
}
Action State::update_contents() {
    Action return_value = Action::CONTINUE;
    for (auto& widget : widgets) {
        if ((return_value = widget->update_content()) != Action::CONTINUE) {
            break;
        }
    }
    return return_value;
}
void State::draw() {
    draw_background_color(rect_px, blueprint->background_color);
    for (auto& widget : widgets) {
        widget->draw();
    }
    draw_border(rect_px, blueprint->border_width_px);
}

std::unique_ptr<State::Widget> State::create_widget_state(const Blueprint::Widget& blueprint) {
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
	} else {
		throw M2FATAL("Implementation");
	}
	return state;
}

Action m2::ui::execute_blocking(const Blueprint *blueprint) {
	return execute_blocking(blueprint, GAME.windowRect);
}

Action m2::ui::execute_blocking(const Blueprint *blueprint, SDL_Rect rect) {
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
	std::unique_ptr<SDL_Texture, m2::SdlTextureDeleter> texture(SDL_CreateTextureFromSurface(GAME.sdlRenderer, surface));
	SDL_FreeSurface(surface);

	Action return_value;

    State state(blueprint);
    state.update_positions(rect);
	if ((return_value = state.update_contents()) != Action::CONTINUE) {
		return return_value;
	}

    m2::Events events;
    while (true) {
        ////////////////////////////////////////////////////////////////////////
        //////////////////////////// EVENT HANDLING ////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        events.clear();
        if (events.gather()) {
            if (events.pop_quit()) {
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

const Blueprint::Widget::Variant command_input_variant = Blueprint::Widget::TextInput{
        .initial_text = "",
        .action_callback = [](std::stringstream& ss) -> Action {
            auto command = ss.str();
	        ss = std::stringstream();
	        GAME.console_output.emplace_back(">> " + command);

            if (std::regex_match(command, std::regex{"editor(\\s.*)?"})) {
				std::smatch match_results;
				if (std::regex_match(command, match_results, std::regex{"editor\\s+(.+)"})) {
					auto load_result = GAME.load_editor(match_results.str(1));
					if (load_result) {
						return Action::RETURN;
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("editor usage:");
					GAME.console_output.emplace_back(".. file_name - open editor with file" );
				}
				return Action::CONTINUE;
			} else if (command == "quit") {
                return Action::QUIT;
            } else if (command.empty()) {
				// Do nothing
			} else {
				GAME.console_output.emplace_back("Hello!");
	            GAME.console_output.emplace_back("Available commands:");
	            GAME.console_output.emplace_back("help - display this help");
	            GAME.console_output.emplace_back("editor - open editor");
	            GAME.console_output.emplace_back("quit - quit game");
            }

	        return Action::CONTINUE;
        }
};
template <unsigned INDEX>
Blueprint::Widget::Variant command_output_variant() {
    return Blueprint::Widget::Text{
        .initial_text = "",
        .alignment = TextAlignment::LEFT,
        .update_callback = []() -> std::pair<Action,std::optional<std::string>> {
            return {Action::CONTINUE, INDEX < GAME.console_output.size() ? GAME.console_output[GAME.console_output.size() - INDEX - 1] : std::string()};
        }
    };
}
const Blueprint m2::ui::console_ui = {
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
