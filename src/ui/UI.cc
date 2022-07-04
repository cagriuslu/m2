#include <m2/ui/UI.h>
#include <m2/Events.h>
#include <m2/Def.h>
#include <m2/Game.hh>
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
        events.clear();
        if (events.gather()) {
            if (events.pop_quit()) {
                return Action::QUIT;
            }
			auto window_resize = events.pop_window_resize();
            if (window_resize) {
				GAME.update_window_dims(window_resize->x, window_resize->y);
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

namespace {
	auto no_string = []() {
		return std::make_pair(Action::CONTINUE, std::optional<std::string>{});
	};
}

const WidgetBlueprint::WidgetBlueprintVariant command_input_variant = wdg::TextInputBlueprint{
        .initial_text = "",
        .action_callback = [](std::stringstream& ss) -> Action {
            Action return_value = Action::CONTINUE;

			static const std::regex editor_regex("editor\\s+(.+)");
			std::smatch match_results;

            auto command = ss.str();
            if (std::regex_match(command, match_results, editor_regex)) {
	            std::filesystem::path file_path{match_results.str(1)};
				auto load_result = GAME.load_editor(file_path);
				if (not load_result) {
					GAME.console_output = { load_result.error() };
				} else {
					return Action::RETURN;
				}
			} else if (command == "quit") {
                return_value = Action::QUIT;
            } else {
	            GAME.console_output = {
		            "Hello!",
		            "Available commands:",
		            "help - display this help",
		            "editor file_name - open editor with file",
		            "quit - quit game"
	            };
            }
            ss = std::stringstream();

            return return_value;
        }
};
template <unsigned INDEX>
WidgetBlueprint::WidgetBlueprintVariant command_output_variant() {
    return wdg::TextBlueprint{
        .initial_text = "",
        .alignment = TextAlignment::LEFT,
        .update_callback = []() -> std::pair<Action,std::optional<std::string>> {
            return {Action::CONTINUE, INDEX < GAME.console_output.size() ? GAME.console_output[INDEX] : std::string()};
        }
    };
}
const UIBlueprint m2::ui::console_ui = {
        .w = 1, .h = 24,
        .widgets = {
	        WidgetBlueprint{
                        .x = 0, .y = 0, .w = 1, .h = 1,
                        .border_width_px = 1,
                        .variant = command_input_variant
                },
	        WidgetBlueprint{.x = 0, .y = 1, .w = 1, .h = 1, .variant = command_output_variant<0>()},
	        WidgetBlueprint{.x = 0, .y = 2, .w = 1, .h = 1, .variant = command_output_variant<1>()},
	        WidgetBlueprint{.x = 0, .y = 3, .w = 1, .h = 1, .variant = command_output_variant<2>()},
	        WidgetBlueprint{.x = 0, .y = 4, .w = 1, .h = 1, .variant = command_output_variant<3>()},
	        WidgetBlueprint{.x = 0, .y = 5, .w = 1, .h = 1, .variant = command_output_variant<4>()},
	        WidgetBlueprint{.x = 0, .y = 6, .w = 1, .h = 1, .variant = command_output_variant<5>()},
	        WidgetBlueprint{.x = 0, .y = 7, .w = 1, .h = 1, .variant = command_output_variant<6>()},
	        WidgetBlueprint{.x = 0, .y = 8, .w = 1, .h = 1, .variant = command_output_variant<7>()},
	        WidgetBlueprint{.x = 0, .y = 9, .w = 1, .h = 1, .variant = command_output_variant<8>()},
	        WidgetBlueprint{.x = 0, .y = 10, .w = 1, .h = 1, .variant = command_output_variant<9>()},
	        WidgetBlueprint{.x = 0, .y = 11, .w = 1, .h = 1, .variant = command_output_variant<10>()},
	        WidgetBlueprint{.x = 0, .y = 12, .w = 1, .h = 1, .variant = command_output_variant<11>()},
	        WidgetBlueprint{.x = 0, .y = 13, .w = 1, .h = 1, .variant = command_output_variant<12>()},
	        WidgetBlueprint{.x = 0, .y = 14, .w = 1, .h = 1, .variant = command_output_variant<13>()},
	        WidgetBlueprint{.x = 0, .y = 15, .w = 1, .h = 1, .variant = command_output_variant<14>()},
	        WidgetBlueprint{.x = 0, .y = 16, .w = 1, .h = 1, .variant = command_output_variant<15>()},
	        WidgetBlueprint{.x = 0, .y = 17, .w = 1, .h = 1, .variant = command_output_variant<16>()},
	        WidgetBlueprint{.x = 0, .y = 18, .w = 1, .h = 1, .variant = command_output_variant<17>()},
	        WidgetBlueprint{.x = 0, .y = 19, .w = 1, .h = 1, .variant = command_output_variant<18>()},
	        WidgetBlueprint{.x = 0, .y = 20, .w = 1, .h = 1, .variant = command_output_variant<19>()},
	        WidgetBlueprint{.x = 0, .y = 21, .w = 1, .h = 1, .variant = command_output_variant<20>()},
	        WidgetBlueprint{.x = 0, .y = 22, .w = 1, .h = 1, .variant = command_output_variant<21>()},
	        WidgetBlueprint{.x = 0, .y = 23, .w = 1, .h = 1, .variant = command_output_variant<22>()}
        }
};

const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_1 = wdg::ImageBlueprint{
	.initial_sprite_index = 1,
	.action_callback = []() {
		fprintf(stderr, "Image action\n");
		return Action::CONTINUE;
	}
};
const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_2 = wdg::TextBlueprint{
	.initial_text = "<"
};
const WidgetBlueprint::WidgetBlueprintVariant editor_right_hud_draw_3 = wdg::TextBlueprint{
	.initial_text = ">"
};
const UIBlueprint editor_right_hud_draw = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		WidgetBlueprint{
			.x = 4, .y = 4, .w = 11, .h = 11,
			.border_width_px = 1,
			.variant = editor_right_hud_draw_1
		},
		WidgetBlueprint{
			.x = 4, .y = 16, .w = 5, .h = 5,
			.border_width_px = 1,
			.variant = editor_right_hud_draw_2
		},
		WidgetBlueprint{
			.x = 10, .y = 16, .w = 5, .h = 5,
			.border_width_px = 1,
			.variant = editor_right_hud_draw_3
		}
	}
};

const WidgetBlueprint::WidgetBlueprintVariant editor_left_hud_1 = wdg::TextBlueprint{
	.initial_text = "Draw",
	.action_callback = []() -> Action {
		GAME.rightHudUIState = UIState(&editor_right_hud_draw);
		GAME.rightHudUIState->update_positions(GAME.rightHudRect);
		fprintf(stderr, "Action callback\n");
		return Action::CONTINUE;
	}
};
const UIBlueprint m2::ui::editor_left_hud = {
	.w = 19, .h = 72,
	.border_width_px = 1,
	.widgets = {
		WidgetBlueprint{
			.x = 4, .y = 4, .w = 11, .h = 3,
			.border_width_px = 1,
			.padding_width_px = 4,
			.variant = editor_left_hud_1
		}
	}
};

const UIBlueprint m2::ui::editor_right_hud_empty = {
	.w = 19, .h = 72,
	.border_width_px = 1
};
