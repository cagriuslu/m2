#include <m2/ui/UI.h>
#include <m2/Events.h>
#include <m2/Game.hh>
#include <m2/SDLUtils.hh>
#include <m2/ui/console/Editor.h>
#include <regex>
#include <filesystem>

using namespace m2::ui;

static SDL_Rect calculate_widget_rect(const SDL_Rect& root_rect_px, unsigned root_w, unsigned root_h, unsigned child_x, unsigned child_y, unsigned child_w, unsigned child_h) {
    auto pixels_per_unit_w = (float)root_rect_px.w / (float)root_w;
    auto pixels_per_unit_h = (float)root_rect_px.h / (float)root_h;
    return SDL_Rect{
            root_rect_px.x + (int)roundf((float)child_x * pixels_per_unit_w),
            root_rect_px.y + (int)roundf((float)child_y * pixels_per_unit_h),
            (int)roundf((float)child_w * pixels_per_unit_w),
            (int)roundf((float)child_h * pixels_per_unit_h)
    };
}

void UIState::draw_background_color(const SDL_Rect& rect, const SDL_Color& color) {
    if (color.a == 0) {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
    } else {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, color.r, color.g, color.b, color.a);
    }
    SDL_RenderFillRect(GAME.sdlRenderer, &rect);
}

void UIState::draw_border(const SDL_Rect& rect, unsigned border_width_px) {
    if (border_width_px) {
        SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(GAME.sdlRenderer, &rect);
    }
}

UIState::UIState() : blueprint(nullptr), rect_px() {}

UIState::UIState(const UIBlueprint* blueprint) : blueprint(blueprint), rect_px({}) {
    for (const auto& widget_blueprint : blueprint->widgets) {
        widgets.push_back(widget_blueprint.get_state());
    }
}

void UIState::update_positions(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
    for (auto& widget_state : widgets) {
        SDL_Rect widget_rect = calculate_widget_rect(rect_px_, blueprint->w, blueprint->h, widget_state->blueprint->x, widget_state->blueprint->y, widget_state->blueprint->w, widget_state->blueprint->h);
        widget_state->update_position(widget_rect);
    }
}

Action UIState::handle_events(Events& events) {
    Action return_value = Action::CONTINUE;
    for (auto& widget : widgets) {
        if ((return_value = widget->handle_events(events)) != Action::CONTINUE) {
            break;
        }
    }
    return return_value;
}

Action UIState::update_contents() {
    Action return_value = Action::CONTINUE;
    for (auto& widget : widgets) {
        if ((return_value = widget->update_content()) != Action::CONTINUE) {
            break;
        }
    }
    return return_value;
}

void UIState::draw() {
    draw_background_color(rect_px, blueprint->background_color);
    for (auto& widget : widgets) {
        widget->draw();
    }
    draw_border(rect_px, blueprint->border_width_px);
}

Action m2::ui::execute_blocking(const UIBlueprint *blueprint) {
	return execute_blocking(blueprint, GAME.windowRect);
}

Action m2::ui::execute_blocking(const UIBlueprint *blueprint, SDL_Rect rect) {
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

    UIState state(blueprint);
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

const WidgetBlueprint::Variant command_input_variant = wdg::TextInputBlueprint{
        .initial_text = "",
        .action_callback = [](std::stringstream& ss) -> Action {
            auto command = ss.str();
	        ss = std::stringstream();
	        GAME.console_output.emplace_back(">> " + command);

            if (std::regex_match(command, std::regex{"editor(\\s.*)?"})) {
	            return m2::ui::con::editor(command);
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
WidgetBlueprint::Variant command_output_variant() {
    return wdg::TextBlueprint{
        .initial_text = "",
        .alignment = TextAlignment::LEFT,
        .update_callback = []() -> std::pair<Action,std::optional<std::string>> {
            return {Action::CONTINUE, INDEX < GAME.console_output.size() ? GAME.console_output[GAME.console_output.size() - INDEX - 1] : std::string()};
        }
    };
}
const UIBlueprint m2::ui::console_ui = {
        .w = 1, .h = 25,
        .widgets = {
	        WidgetBlueprint{.x = 0, .y = 0, .w = 1, .h = 1, .variant = command_output_variant<23>()},
	        WidgetBlueprint{.x = 0, .y = 1, .w = 1, .h = 1, .variant = command_output_variant<22>()},
	        WidgetBlueprint{.x = 0, .y = 2, .w = 1, .h = 1, .variant = command_output_variant<21>()},
	        WidgetBlueprint{.x = 0, .y = 3, .w = 1, .h = 1, .variant = command_output_variant<20>()},
	        WidgetBlueprint{.x = 0, .y = 4, .w = 1, .h = 1, .variant = command_output_variant<19>()},
	        WidgetBlueprint{.x = 0, .y = 5, .w = 1, .h = 1, .variant = command_output_variant<18>()},
	        WidgetBlueprint{.x = 0, .y = 6, .w = 1, .h = 1, .variant = command_output_variant<17>()},
	        WidgetBlueprint{.x = 0, .y = 7, .w = 1, .h = 1, .variant = command_output_variant<16>()},
	        WidgetBlueprint{.x = 0, .y = 8, .w = 1, .h = 1, .variant = command_output_variant<15>()},
	        WidgetBlueprint{.x = 0, .y = 9, .w = 1, .h = 1, .variant = command_output_variant<14>()},
	        WidgetBlueprint{.x = 0, .y = 10, .w = 1, .h = 1, .variant = command_output_variant<13>()},
	        WidgetBlueprint{.x = 0, .y = 11, .w = 1, .h = 1, .variant = command_output_variant<12>()},
	        WidgetBlueprint{.x = 0, .y = 12, .w = 1, .h = 1, .variant = command_output_variant<11>()},
	        WidgetBlueprint{.x = 0, .y = 13, .w = 1, .h = 1, .variant = command_output_variant<10>()},
	        WidgetBlueprint{.x = 0, .y = 14, .w = 1, .h = 1, .variant = command_output_variant<9>()},
	        WidgetBlueprint{.x = 0, .y = 15, .w = 1, .h = 1, .variant = command_output_variant<8>()},
	        WidgetBlueprint{.x = 0, .y = 16, .w = 1, .h = 1, .variant = command_output_variant<7>()},
	        WidgetBlueprint{.x = 0, .y = 17, .w = 1, .h = 1, .variant = command_output_variant<6>()},
	        WidgetBlueprint{.x = 0, .y = 18, .w = 1, .h = 1, .variant = command_output_variant<5>()},
	        WidgetBlueprint{.x = 0, .y = 19, .w = 1, .h = 1, .variant = command_output_variant<4>()},
	        WidgetBlueprint{.x = 0, .y = 20, .w = 1, .h = 1, .variant = command_output_variant<3>()},
	        WidgetBlueprint{.x = 0, .y = 21, .w = 1, .h = 1, .variant = command_output_variant<2>()},
	        WidgetBlueprint{.x = 0, .y = 22, .w = 1, .h = 1, .variant = command_output_variant<1>()},
	        WidgetBlueprint{.x = 0, .y = 23, .w = 1, .h = 1, .variant = command_output_variant<0>()},
	        WidgetBlueprint{
		        .x = 0, .y = 24, .w = 1, .h = 1,
				.background_color = SDL_Color{27, 27, 27, 255},
		        .variant = command_input_variant
	        }
        }
};
