#include <m2/ui/State.h>
#include <m2/Game.h>
#include <m2/ui/Blueprint.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/Image.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/ui/widget/NestedUi.h>
#include <m2/ui/widget/ProgressBar.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/sdl/Detail.hh>
#include <regex>

using namespace m2;
using namespace m2::ui;

State::State(const Blueprint* blueprint) : blueprint(blueprint) {
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
	if (enabled) {
		for (auto& widget : widgets) {
			if (widget->enabled && (return_value = widget->handle_events(events)) != Action::CONTINUE) {
				break;
			}
		}
	}
	return return_value;
}

Action State::update_contents() {
	Action return_value = Action::CONTINUE;
	if (enabled) {
		for (auto& widget : widgets) {
			if (widget->enabled && (return_value = widget->update_content()) != Action::CONTINUE) {
				break;
			}
		}
	}
	return return_value;
}

void State::draw() {
	if (enabled) {
		Widget::draw_background_color(rect_px, blueprint->background_color);
		for (auto& widget : widgets) {
			if (widget->enabled) {
				widget->draw();
			}
		}
		Widget::draw_border(rect_px, blueprint->border_width_px);
	}
}

std::unique_ptr<Widget> State::create_widget_state(const WidgetBlueprint& blueprint) {
	std::unique_ptr<Widget> state;

	using namespace m2::ui::widget;
	if (std::holds_alternative<NestedUiBlueprint>(blueprint.variant)) {
		state = std::make_unique<NestedUi>(&blueprint);
	} else if (std::holds_alternative<TextBlueprint>(blueprint.variant)) {
		state = std::make_unique<Text>(&blueprint);
	} else if (std::holds_alternative<TextInputBlueprint>(blueprint.variant)) {
		state = std::make_unique<TextInput>(&blueprint);
	} else if (std::holds_alternative<ImageBlueprint>(blueprint.variant)) {
		state = std::make_unique<Image>(&blueprint);
	} else if (std::holds_alternative<ProgressBarBlueprint>(blueprint.variant)) {
		state = std::make_unique<ProgressBar>(&blueprint);
	} else if (std::holds_alternative<ImageSelectionBlueprint>(blueprint.variant)) {
		state = std::make_unique<ImageSelection>(&blueprint);
	} else if (std::holds_alternative<TextSelectionBlueprint>(blueprint.variant)) {
		state = std::make_unique<TextSelection>(&blueprint);
	} else if (std::holds_alternative<IntegerSelectionBlueprint>(blueprint.variant)) {
		state = std::make_unique<IntegerSelection>(&blueprint);
	} else if (std::holds_alternative<CheckboxWithTextBlueprint>(blueprint.variant)) {
		state = std::make_unique<CheckboxWithText>(&blueprint);
	} else {
		throw M2FATAL("Implementation");
	}

	return state;
}

SDL_Rect State::calculate_widget_rect(
		const SDL_Rect& root_rect_px, unsigned root_w, unsigned root_h,
		int child_x, int child_y, unsigned child_w, unsigned child_h) {
	auto pixels_per_unit_w = (float)root_rect_px.w / (float)root_w;
	auto pixels_per_unit_h = (float)root_rect_px.h / (float)root_h;
	return SDL_Rect{
			root_rect_px.x + (int)roundf((float)child_x * pixels_per_unit_w),
			root_rect_px.y + (int)roundf((float)child_y * pixels_per_unit_h),
			(int)roundf((float)child_w * pixels_per_unit_w),
			(int)roundf((float)child_h * pixels_per_unit_h)
	};
}

m2::ui::Action m2::ui::execute_blocking(const Blueprint *blueprint) {
	return execute_blocking(blueprint, GAME.dimensions().window);
}

m2::ui::Action m2::ui::execute_blocking(const Blueprint *blueprint, SDL_Rect rect) {
	auto execute_start_ticks = sdl::get_ticks();

	// Save relation to window, use in case of resize
	const SDL_Rect& winrect = GAME.dimensions().window;
	auto relation_to_window = SDL_FRect{
			(float)(rect.x - winrect.x) / (float)winrect.w,
			(float)(rect.y - winrect.y) / (float)winrect.h,
			(float)rect.w / (float)winrect.w,
			(float)rect.h / (float)winrect.h,
	};

	// Get screenshot
	int w, h;
	SDL_GetRendererOutputSize(GAME.renderer, &w, &h);
	auto* surface = SDL_CreateRGBSurface(0, w, h, 24, 0xFF, 0xFF00, 0xFF0000, 0);
	SDL_RenderReadPixels(GAME.renderer, nullptr, SDL_PIXELFORMAT_RGB24, surface->pixels, surface->pitch);
	std::unique_ptr<SDL_Texture, sdl::TextureDeleter> texture(SDL_CreateTextureFromSurface(GAME.renderer, surface));
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
			if (events.pop_key_press(Key::CONSOLE)) {
				auto act = execute_blocking(&console_ui);
				if (act != Action::CONTINUE) {
					return act;
				}
			}
			auto window_resize = events.pop_window_resize();
			if (window_resize) {
				GAME.recalculate_dimensions(window_resize->x, window_resize->y);
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
		SDL_SetRenderDrawColor(GAME.renderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME.renderer);
		SDL_RenderCopy(GAME.renderer, texture.get(), nullptr, nullptr);
		state.draw();
		// Present
		SDL_RenderPresent(GAME.renderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
	}
}

const WidgetBlueprint::Variant command_input_variant = widget::TextInputBlueprint{
		.initial_text = "",
		.action_callback = [](std::stringstream& ss) -> Action {
			auto command = ss.str();
			ss = std::stringstream();
			GAME.console_output.emplace_back(">> " + command);

			if (std::regex_match(command, std::regex{"ledit(\\s.*)?"})) {
				std::smatch match_results;
				if (std::regex_match(command, match_results, std::regex{"ledit\\s+(.+)"})) {
					auto load_result = GAME.load_level_editor(match_results.str(1));
					if (load_result) {
						return Action::RETURN;
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("ledit usage:");
					GAME.console_output.emplace_back(".. file_name - open level editor with file");
				}
				return Action::CONTINUE;
			} else if (std::regex_match(command, std::regex{"pedit(\\s.*)?"})) {
				std::smatch match_results;
				if (std::regex_match(command, match_results, std::regex{R"(pedit\s+([0-9]+)\s+([0-9]+)\s+(.+))"})) {
					auto x_offset = strtol(match_results.str(1).c_str(), nullptr, 0);
					auto y_offset = strtol(match_results.str(2).c_str(), nullptr, 0);
					auto load_result = GAME.load_pixel_editor(match_results.str(3), (int) x_offset, (int) y_offset);
					if (load_result) {
						return Action::RETURN;
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("pedit usage:");
					GAME.console_output.emplace_back(".. x_offset y_offset file_name - open pixel editor with file");
				}
				return Action::CONTINUE;
			} else if (command == "quit") {
				return Action::QUIT;
			} else if (command.empty()) {
				// Do nothing
			} else {
				GAME.console_output.emplace_back("Available commands:");
				GAME.console_output.emplace_back("help - display this help");
				GAME.console_output.emplace_back("ledit - open level editor");
				GAME.console_output.emplace_back("pedit - open pixel editor");
				GAME.console_output.emplace_back("quit - quit game");
			}

			return Action::CONTINUE;
		}
};

template <unsigned INDEX>
widget::TextBlueprint command_output_variant() {
	return {
			.initial_text = "",
			.alignment = TextAlignment::LEFT,
			.update_callback = []() -> std::pair<Action,std::optional<std::string>> {
				return {Action::CONTINUE, INDEX < GAME.console_output.size() ? GAME.console_output[GAME.console_output.size() - INDEX - 1] : std::string()};
			}
	};
}

const Blueprint m2::ui::console_ui = {
		.w = 1, .h = 25,
		.border_width_px = 0,
		.widgets = {
				WidgetBlueprint{.x = 0, .y = 0, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<23>()},
				WidgetBlueprint{.x = 0, .y = 1, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<22>()},
				WidgetBlueprint{.x = 0, .y = 2, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<21>()},
				WidgetBlueprint{.x = 0, .y = 3, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<20>()},
				WidgetBlueprint{.x = 0, .y = 4, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<19>()},
				WidgetBlueprint{.x = 0, .y = 5, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<18>()},
				WidgetBlueprint{.x = 0, .y = 6, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<17>()},
				WidgetBlueprint{.x = 0, .y = 7, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<16>()},
				WidgetBlueprint{.x = 0, .y = 8, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<15>()},
				WidgetBlueprint{.x = 0, .y = 9, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<14>()},
				WidgetBlueprint{.x = 0, .y = 10, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<13>()},
				WidgetBlueprint{.x = 0, .y = 11, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<12>()},
				WidgetBlueprint{.x = 0, .y = 12, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<11>()},
				WidgetBlueprint{.x = 0, .y = 13, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<10>()},
				WidgetBlueprint{.x = 0, .y = 14, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<9>()},
				WidgetBlueprint{.x = 0, .y = 15, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<8>()},
				WidgetBlueprint{.x = 0, .y = 16, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<7>()},
				WidgetBlueprint{.x = 0, .y = 17, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<6>()},
				WidgetBlueprint{.x = 0, .y = 18, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<5>()},
				WidgetBlueprint{.x = 0, .y = 19, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<4>()},
				WidgetBlueprint{.x = 0, .y = 20, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<3>()},
				WidgetBlueprint{.x = 0, .y = 21, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<2>()},
				WidgetBlueprint{.x = 0, .y = 22, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<1>()},
				WidgetBlueprint{.x = 0, .y = 23, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<0>()},
				WidgetBlueprint{
						.x = 0, .y = 24, .w = 1, .h = 1,
						.background_color = SDL_Color{27, 27, 27, 255},
						.variant = command_input_variant
				}
		}
};

const Blueprint m2::ui::message_box_ui = {
		.w = 1, .h = 1,
		.border_width_px = 0,
		.widgets = {
				WidgetBlueprint{
						.initially_enabled = false,
						.x = 0, .y = 0, .w = 1, .h = 1,
						.border_width_px = 0,
						.background_color = SDL_Color{127, 127, 127, 127},
						.variant = widget::TextBlueprint{
								.alignment = TextAlignment::LEFT,
								.update_callback = []() {
									return std::make_pair(Action::CONTINUE, LEVEL.message);
								}
						}
				}
		}
};