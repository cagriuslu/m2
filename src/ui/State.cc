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
#include <m2/sheet_editor/Ui.h>
#include <m2/sdl/Detail.hh>
#include <regex>
#include <ranges>
#include <m2/String.h>

using namespace m2;
using namespace m2::ui;

namespace {
	// Filters
	constexpr auto is_widget_enabled = [](const auto& w) { return w->enabled; };
	constexpr auto is_widget_focused = [](const auto& w) { return w->focused; };
	// Actions
	constexpr auto draw_widget = [](const auto& w) { w->draw(); };
}

State::State(const Blueprint* blueprint) : blueprint(blueprint) {
	for (const auto& widget_blueprint : blueprint->widgets) {
		// Create widget
		widgets.push_back(create_widget_state(widget_blueprint));
		// Check if focus is requested
		if (widget_blueprint.initially_focused) {
			set_widget_focus_state(*widgets.back(), true);
		}
	}
}

State::~State() {
	clear_focus();
}

Action State::execute() {
	return execute(GAME.dimensions().window);
}

Action State::execute(SDL_Rect rect) {
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

	update_positions(rect);
	if ((return_value = update_contents()) != Action::CONTINUE) {
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
			if (events.pop_key_press(Key::CONSOLE) && blueprint != &console_ui) { // Do not open console on top of console
				LOG_INFO("Opening console");
				auto action = execute_blocking(&console_ui);
				if (action == Action::RETURN) {
					// Continue with the prev UI
				} else if (action == Action::CLEAR_STACK || action == Action::QUIT) {
					return action;
				} else {
					LOG_WARN("Console returned unexpected action", (int) action);
				}
			}
			auto window_resize = events.pop_window_resize();
			if (window_resize) {
				GAME.recalculate_dimensions(window_resize->x, window_resize->y);
				update_positions(SDL_Rect{
						(int)round((float)winrect.x + relation_to_window.x * (float)winrect.w),
						(int)round((float)winrect.y + relation_to_window.y * (float)winrect.h),
						(int)round(relation_to_window.w * (float)winrect.w),
						(int)round(relation_to_window.h * (float)winrect.h)
				});
			}
			if ((return_value = handle_events(events)) != Action::CONTINUE) {
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
		if ((return_value = update_contents()) != Action::CONTINUE) {
			GAME.add_pause_ticks(sdl::get_ticks_since(execute_start_ticks));
			return return_value;
		}
		// Clear screen
		SDL_SetRenderDrawColor(GAME.renderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME.renderer);
		SDL_RenderCopy(GAME.renderer, texture.get(), nullptr, nullptr);
		draw();
		// Present
		SDL_RenderPresent(GAME.renderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
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
	// Return if State not enabled
	if (!enabled) {
		return Action::CONTINUE;
	}

	for (auto& widget : widgets | std::views::filter(is_widget_enabled)) {
		switch (auto action = widget->handle_events(events); action) {
			case Action::CONTINUE:
				continue;
			case Action::GAIN_FOCUS:
			case Action::LOSE_FOCUS:
				set_widget_focus_state(*widget, action == Action::GAIN_FOCUS);
				break;
			default:
				return action;
		}
	}
	return Action::CONTINUE;
}

Action State::update_contents() {
	// Return if State not enabled
	if (!enabled) {
		return Action::CONTINUE;
	}

	for (auto& widget : widgets | std::views::filter(is_widget_enabled)) {
		switch (auto action = widget->update_content(); action) {
			case Action::CONTINUE:
			case Action::GAIN_FOCUS:
			case Action::LOSE_FOCUS:
				continue;
			default:
				return action;
		}
	}
	return Action::CONTINUE;
}

void State::draw() {
	// Return if State not enabled
	if (!enabled) {
		return;
	}

	Widget::draw_background_color(rect_px, blueprint->background_color);
	std::ranges::for_each(widgets | std::views::filter(is_widget_enabled), draw_widget);
	Widget::draw_border(rect_px, blueprint->border_width_px);
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

void State::set_widget_focus_state(Widget& w, bool focus_state) {
	if (focus_state) {
		if (!w.focused) {
			// Clear existing focus
			clear_focus();
			// Set focus
			w.focused = true;
			w.focus_changed();
		}
	} else {
		// Reset focus
		if (w.focused) {
			w.focused = false;
			w.focus_changed();
		}
	}
}

void State::clear_focus() {
	// Check if there's an already focused widget
	std::ranges::for_each(
			widgets | std::views::filter(is_widget_focused),
			[&](const auto& it) { set_widget_focus_state(*it, false); }
	);
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
	State state{blueprint};
	return state.execute(rect);
}

m2::ui::Widget* m2::ui::find_text_widget(State& state, const std::string& text) {
	auto it = std::ranges::find_if(state.widgets,
			// Predicate
			[=](const auto* blueprint) {
				const auto* text_variant = std::get_if<ui::widget::TextBlueprint>(&blueprint->variant);
				// If widget is Text and the button is labelled correctly
				return text_variant && text_variant->initial_text == text;
			},
			// Projection
			[](const auto& unique_blueprint) { return unique_blueprint->blueprint; });

	return (it != state.widgets.end()) ? it->get() : nullptr;
}

const WidgetBlueprint::Variant command_input_variant = widget::TextInputBlueprint{
		.initial_text = "",
		.action_callback = [](std::stringstream& ss) -> Action {
			auto command = ss.str(); // Get command
			ss = std::stringstream(); // Clear the text input
			GAME.console_output.emplace_back(">> " + command);

			if (std::regex_match(command, std::regex{"ledit(\\s.*)?"})) {
				std::smatch match_results;
				if (std::regex_match(command, match_results, std::regex{"ledit\\s+(.+)"})) {
					auto load_result = GAME.load_level_editor(match_results.str(1));
					if (load_result) {
						return Action::CLEAR_STACK;
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
						return Action::CLEAR_STACK;
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("pedit usage:");
					GAME.console_output.emplace_back(".. x_offset y_offset file_name - open pixel editor with file");
				}
				return Action::CONTINUE;
			} else if (std::regex_match(command, std::regex{"sedit(\\s.*)?"})) {
				std::smatch match_results;
				if (std::regex_match(command, match_results, std::regex{R"(sedit(\s.*)?)"})) {
					auto trimmed_arg = string::trim(match_results.str(1));
					auto path = trimmed_arg.empty() ? (GAME.game_resource_dir / "SpriteSheets.json").string() : trimmed_arg;
					auto load_result = GAME.load_sheet_editor(path);
					if (load_result) {
						// Execute main menu the first time the sheet editor is run
						auto main_menu_result = execute_blocking(&m2::ui::sheet_editor_main_menu);
						return main_menu_result == Action::RETURN ? Action::CLEAR_STACK : main_menu_result;
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("sedit usage:");
					GAME.console_output.emplace_back(".. file_name - open sheet editor with file");
				}
				return Action::CONTINUE;
			} else if (std::regex_match(command, std::regex{"set(\\s.*)?"})) {
				std::smatch match_results;
				if (std::regex_match(command, match_results, std::regex{R"(set\s+([_a-zA-Z]+)\s+([a-zA-Z0-9]+))"})) {
					auto parameter = match_results.str(1);
					if (parameter == "game_height") {
						auto new_game_height = I(strtol(match_results.str(2).c_str(), nullptr, 0));
						GAME.recalculate_dimensions(GAME.dimensions().window.w, GAME.dimensions().window.h, new_game_height);
						return Action::CONTINUE;
					} else {
						GAME.console_output.emplace_back("Unknown parameter");
					}
				} else {
					// TODO print help
				}
			} else if (command == "quit") {
				return Action::QUIT;
			} else if (command == "close") {
				return Action::RETURN;
			} else if (command.empty()) {
				// Do nothing
			} else {
				GAME.console_output.emplace_back("Available commands:");
				GAME.console_output.emplace_back("help - display this help");
				GAME.console_output.emplace_back("ledit - open level editor");
				GAME.console_output.emplace_back("pedit - open pixel editor");
				GAME.console_output.emplace_back("sedit - open sheet editor");
				GAME.console_output.emplace_back("set - set game variable");
				GAME.console_output.emplace_back("close - close the console");
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
		.background_color = {0, 0, 0, 255},
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
						.initially_focused = true,
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
