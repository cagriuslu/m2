#include <m2/ui/UI.h>
#include <m2/Event.hh>
#include <m2/Def.h>
#include <m2/Game.hh>

using namespace m2::ui;

static SDL_Rect calculate_element_rect(const SDL_Rect& root_rect_px, unsigned root_w, unsigned root_h, unsigned child_x, unsigned child_y, unsigned child_w, unsigned child_h) {
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
    for (const auto& element_blueprint : blueprint->elements) {
        elements.push_back(element_blueprint.get_state());
    }
}

void UIState::update_positions(const SDL_Rect &rect_px_) {
    this->rect_px = rect_px_;
    for (auto& element_state : elements) {
        SDL_Rect element_rect = calculate_element_rect(rect_px_, blueprint->w, blueprint->h, element_state->blueprint->x, element_state->blueprint->y, element_state->blueprint->w, element_state->blueprint->h);
        element_state->update_position(element_rect);
    }
}

Action UIState::handle_events(const Events& events) {
    Action return_value = Action::CONTINUE;
    for (auto& element : elements) {
        if ((return_value = element->handle_events(events)) != Action::CONTINUE) {
            break;
        }
    }
    return return_value;
}

Action UIState::update_contents() {
    Action return_value = Action::CONTINUE;
    for (auto& element : elements) {
        if ((return_value = element->update_content()) != Action::CONTINUE) {
            break;
        }
    }
    return return_value;
}

void UIState::draw() {
    draw_background_color(rect_px, blueprint->background_color);
    for (auto& element : elements) {
        element->draw();
    }
    draw_border(rect_px, blueprint->border_width_px);
}

Action m2::ui::execute_blocking(const UIBlueprint *blueprint) {
	Action return_value;

    UIState state(blueprint);
    state.update_positions(GAME.windowRect);
	if ((return_value = state.update_contents()) != Action::CONTINUE) {
		return return_value;
	}

    m2::Events events;
    while (true) {
        ////////////////////////////////////////////////////////////////////////
        //////////////////////////// EVENT HANDLING ////////////////////////////
        ////////////////////////////////////////////////////////////////////////
        if (events.gather()) {
            if (events.quit) {
                return Action::QUIT;
            }
            if (events.window_resized) {
                Game_UpdateWindowDimensions(events.window_dimensions.x, events.window_dimensions.y);
                state.update_positions(GAME.windowRect);
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
        state.draw();
        // Present
        SDL_RenderPresent(GAME.sdlRenderer);
        /////////////////////////// END OF GRAPHICS ////////////////////////////
        ////////////////////////////////////////////////////////////////////////
    }
}

const ElementBlueprint::ElementBlueprintVariant command_input_variant = element::TextInputBlueprint{
        .initial_text = "",
        .action_callback = [](std::stringstream& ss) -> Action {
            Action return_value = Action::CONTINUE;

            auto command = ss.str();
            if (command == "help") {
                GAME.console_output = {
                    "Hello!",
                    "Available commands:",
                    "HELP - display this help",
                    "QUIT - quit game"
                };
            } else if (command == "quit") {
                return_value = Action::QUIT;
            } else {
                GAME.console_output = {"unknown command"};
            }
            ss = std::stringstream();

            return return_value;
        }
};
template <unsigned INDEX>
ElementBlueprint::ElementBlueprintVariant command_output_variant() {
    return element::TextBlueprint{
        .initial_text = "",
        .alignment = TextAlignment::LEFT,
        .update_callback = []() -> std::pair<Action,std::optional<std::string>> {
            return {Action::CONTINUE, INDEX < GAME.console_output.size() ? GAME.console_output[INDEX] : std::string()};
        }
    };
}
const UIBlueprint m2::ui::console_ui = {
        .w = 1, .h = 24,
        .elements = {
                ElementBlueprint{
                        .x = 0, .y = 0, .w = 1, .h = 1,
                        .border_width_px = 1,
                        .variant = command_input_variant
                },
                ElementBlueprint{.x = 0, .y = 1, .w = 1, .h = 1, .variant = command_output_variant<0>()},
                ElementBlueprint{.x = 0, .y = 2, .w = 1, .h = 1, .variant = command_output_variant<1>()},
                ElementBlueprint{.x = 0, .y = 3, .w = 1, .h = 1, .variant = command_output_variant<2>()},
                ElementBlueprint{.x = 0, .y = 4, .w = 1, .h = 1, .variant = command_output_variant<3>()},
                ElementBlueprint{.x = 0, .y = 5, .w = 1, .h = 1, .variant = command_output_variant<4>()},
                ElementBlueprint{.x = 0, .y = 6, .w = 1, .h = 1, .variant = command_output_variant<5>()},
                ElementBlueprint{.x = 0, .y = 7, .w = 1, .h = 1, .variant = command_output_variant<6>()},
                ElementBlueprint{.x = 0, .y = 8, .w = 1, .h = 1, .variant = command_output_variant<7>()},
                ElementBlueprint{.x = 0, .y = 9, .w = 1, .h = 1, .variant = command_output_variant<8>()},
                ElementBlueprint{.x = 0, .y = 10, .w = 1, .h = 1, .variant = command_output_variant<9>()},
                ElementBlueprint{.x = 0, .y = 11, .w = 1, .h = 1, .variant = command_output_variant<10>()},
                ElementBlueprint{.x = 0, .y = 12, .w = 1, .h = 1, .variant = command_output_variant<11>()},
                ElementBlueprint{.x = 0, .y = 13, .w = 1, .h = 1, .variant = command_output_variant<12>()},
                ElementBlueprint{.x = 0, .y = 14, .w = 1, .h = 1, .variant = command_output_variant<13>()},
                ElementBlueprint{.x = 0, .y = 15, .w = 1, .h = 1, .variant = command_output_variant<14>()},
                ElementBlueprint{.x = 0, .y = 16, .w = 1, .h = 1, .variant = command_output_variant<15>()},
                ElementBlueprint{.x = 0, .y = 17, .w = 1, .h = 1, .variant = command_output_variant<16>()},
                ElementBlueprint{.x = 0, .y = 18, .w = 1, .h = 1, .variant = command_output_variant<17>()},
                ElementBlueprint{.x = 0, .y = 19, .w = 1, .h = 1, .variant = command_output_variant<18>()},
                ElementBlueprint{.x = 0, .y = 20, .w = 1, .h = 1, .variant = command_output_variant<19>()},
                ElementBlueprint{.x = 0, .y = 21, .w = 1, .h = 1, .variant = command_output_variant<20>()},
                ElementBlueprint{.x = 0, .y = 22, .w = 1, .h = 1, .variant = command_output_variant<21>()},
                ElementBlueprint{.x = 0, .y = 23, .w = 1, .h = 1, .variant = command_output_variant<22>()}
        }
};
