#include <m2/ui/State.h>
#include <m2/Game.h>
#include <m2/ui/Blueprint.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/Image.h>
#include <m2/ui/widget/Hidden.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/ui/widget/NestedUi.h>
#include <m2/ui/widget/ProgressBar.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/sheet_editor/Ui.h>
#include <m2/sdl/Detail.h>
#include <regex>
#include <ranges>
#include <m2/String.h>

using namespace m2;
using namespace m2::ui;

namespace {
	// Filters
	constexpr auto is_widget_enabled = [](const auto &w) { return w->enabled; };
	constexpr auto is_widget_focused = [](const auto &w) { return w->focused; };
	// Actions
	constexpr auto draw_widget = [](const auto &w) { w->on_draw(); };
}

State::State(const Blueprint *blueprint) : blueprint(blueprint) {
	for (const auto &widget_blueprint: blueprint->widgets) {
		// Create widget
		widgets.push_back(create_widget_state(widget_blueprint));
		// Check if focus is requested
		if (widget_blueprint.initially_focused) {
			set_widget_focus_state(*widgets.back(), true);
		}
	}
}

Action State::create_execute_sync(const Blueprint *blueprint) {
	return create_execute_sync(blueprint, GAME.dimensions().window);
}

Action State::create_execute_sync(const Blueprint *blueprint, const RectI rect) {
	// Check if there are other blocking UIs
	if (GAME.ui_begin_ticks) {
		// Execute state without keeping time
		State state{blueprint};
		return state.execute(rect);
	} else {
		// Save begin ticks for later and other nested UIs
		GAME.ui_begin_ticks = sdl::get_ticks();
		// Execute state
		State state{blueprint};
		const auto action = state.execute(rect);
		// Add pause ticks
		GAME.add_pause_ticks(sdl::get_ticks_since(*GAME.ui_begin_ticks));
		GAME.ui_begin_ticks.reset();
		return action;
	}
}

State::~State() {
	clear_focus();
}

Action State::execute(const RectI rect) {
	DEBUG_FN();

	// Save relation to window, use in case of resize
	const auto& winrect = GAME.dimensions().window;
	const auto relation_to_window = RectF{
			static_cast<float>(rect.x - winrect.x) / static_cast<float>(winrect.w),
			static_cast<float>(rect.y - winrect.y) / static_cast<float>(winrect.h),
			static_cast<float>(rect.w) / static_cast<float>(winrect.w),
			static_cast<float>(rect.h) / static_cast<float>(winrect.h)
	};

	// Get screenshot
	const auto screen_capture = sdl::capture_screen_as_texture();

	// Update initial positions
	update_positions(rect);

	// Update initial contents
	if (const Action return_value = update_contents(); return_value != Action::CONTINUE) {
		return return_value;
	}

	Events events;
	while (true) {
		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		events.clear();
		if (events.gather()) {
			// Handle quit action
			if (events.pop_quit()) {
				return Action::QUIT;
			}

			// Handle console action
			if (events.pop_key_press(Key::CONSOLE) &&
					blueprint != &console_ui) { // Do not open console on top of console
				LOG_INFO("Opening console");
				if (auto action = create_execute_sync(&console_ui); action == Action::RETURN) {
					// Continue with the prev UI
					LOG_DEBUG("Console returned");
				} else if (action == Action::CLEAR_STACK || action == Action::QUIT) {
					LOG_DEBUG("Console returned", static_cast<int>(action));
					return action;
				} else {
					LOG_WARN("Console returned unexpected action", static_cast<int>(action));
				}
			}

			// Handle resize action
			if (const auto window_resize = events.pop_window_resize(); window_resize) {
				GAME.recalculate_dimensions(window_resize->x, window_resize->y);
				update_positions(RectI{
						static_cast<int>(round(static_cast<float>(winrect.x) + relation_to_window.x * static_cast<float>(winrect.w))),
						static_cast<int>(round(static_cast<float>(winrect.y) + relation_to_window.y * static_cast<float>(winrect.h))),
						static_cast<int>(round(relation_to_window.w * static_cast<float>(winrect.w))),
						static_cast<int>(round(relation_to_window.h * static_cast<float>(winrect.h)))
				});
			}

			// Handle events
			if (const Action return_value = handle_events(events); return_value != Action::CONTINUE) {
				return return_value;
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Update contents
		if (const Action return_value = update_contents(); return_value != Action::CONTINUE) {
			return return_value;
		}

		// Clear screen
		SDL_SetRenderDrawColor(GAME.renderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME.renderer);
		SDL_RenderCopy(GAME.renderer, screen_capture.get(), nullptr, nullptr);

		// Draw UI elements
		draw();

		// Present
		SDL_RenderPresent(GAME.renderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
	}
}

void State::update_positions(const RectI& rect) {
	this->rect_px = rect;
	for (const auto &widget_state: widgets) {
		auto widget_rect = calculate_widget_rect(rect, blueprint->w, blueprint->h, widget_state->blueprint->x,
				widget_state->blueprint->y, widget_state->blueprint->w, widget_state->blueprint->h);
		widget_state->on_position_update(widget_rect);
	}
}

Action State::handle_events(Events &events) {
	// Return if State not enabled
	if (!enabled) {
		return Action::CONTINUE;
	}

	if (blueprint->cancellable && events.pop_key_press(Key::MENU)) {
		return Action::RETURN;
	}

	for (auto &widget: widgets | std::views::filter(is_widget_enabled)) {
		switch (const auto action = widget->on_event(events); action) {
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

	for (const auto &widget: widgets | std::views::filter(is_widget_enabled)) {
		switch (const auto action = widget->on_update(); action) {
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

std::unique_ptr<Widget> State::create_widget_state(const WidgetBlueprint &widget_blueprint) {
	std::unique_ptr<Widget> state;

	using namespace m2::ui::widget;
	if (std::holds_alternative<NestedUiBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<NestedUi>(this, &widget_blueprint);
	} else if (std::holds_alternative<HiddenBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<Hidden>(this, &widget_blueprint);
	} else if (std::holds_alternative<TextBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<Text>(this, &widget_blueprint);
	} else if (std::holds_alternative<TextInputBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<TextInput>(this, &widget_blueprint);
	} else if (std::holds_alternative<ImageBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<Image>(this, &widget_blueprint);
	} else if (std::holds_alternative<ProgressBarBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<ProgressBar>(this, &widget_blueprint);
	} else if (std::holds_alternative<ImageSelectionBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<ImageSelection>(this, &widget_blueprint);
	} else if (std::holds_alternative<TextSelectionBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<TextSelection>(this, &widget_blueprint);
	} else if (std::holds_alternative<IntegerSelectionBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<IntegerSelection>(this, &widget_blueprint);
	} else if (std::holds_alternative<CheckboxWithTextBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<CheckboxWithText>(this, &widget_blueprint);
	} else {
		throw M2FATAL("Implementation");
	}

	return state;
}

void State::set_widget_focus_state(Widget &w, const bool state) {
	if (state) {
		if (!w.focused) {
			// Clear existing focus
			clear_focus();
			// Set focus
			w.focused = true;
			w.on_focus_change();
		}
	} else {
		// Reset focus
		if (w.focused) {
			w.focused = false;
			w.on_focus_change();
		}
	}
}

void State::clear_focus() {
	// Check if there's an already focused widget
	std::ranges::for_each(
			widgets | std::views::filter(is_widget_focused),
			[&](const auto &it) { set_widget_focus_state(*it, false); }
	);
}

RectI ui::calculate_widget_rect(const RectI& root_rect_px, const unsigned root_w, const unsigned root_h, const int child_x, const int child_y, const unsigned child_w, const unsigned child_h) {
	const auto pixels_per_unit_w = static_cast<float>(root_rect_px.w) / static_cast<float>(root_w);
	const auto pixels_per_unit_h = static_cast<float>(root_rect_px.h) / static_cast<float>(root_h);
	return RectI{
			root_rect_px.x + static_cast<int>(roundf(static_cast<float>(child_x) * pixels_per_unit_w)),
			root_rect_px.y + static_cast<int>(roundf(static_cast<float>(child_y) * pixels_per_unit_h)),
			static_cast<int>(roundf(static_cast<float>(child_w) * pixels_per_unit_w)),
			static_cast<int>(roundf(static_cast<float>(child_h) * pixels_per_unit_h))
	};
}

m2::ui::Widget *m2::ui::find_text_widget(State &state, const std::string &text) {
	const auto it = std::ranges::find_if(state.widgets,
			// Predicate
			[=](const auto *blueprint) {
				const auto *text_variant = std::get_if<ui::widget::TextBlueprint>(&blueprint->variant);
				// If widget is Text and the button is labelled correctly
				return text_variant && text_variant->initial_text == text;
			},
			// Projection
			[](const auto &unique_blueprint) { return unique_blueprint->blueprint; });

	return (it != state.widgets.end()) ? it->get() : nullptr;
}

const WidgetBlueprint::Variant command_input_variant = widget::TextInputBlueprint{
		.initial_text = "",
		.on_action = [](const widget::TextInput &self) -> std::pair<Action, std::optional<std::string>> {
			const auto &command = self.text_input();
			GAME.console_output.emplace_back(">> " + command);

			if (std::regex_match(command, std::regex{"ledit(\\s.*)?"})) {
				if (std::smatch match_results; std::regex_match(command, match_results, std::regex{"ledit\\s+(.+)"})) {
					auto load_result = GAME.load_level_editor(GAME.levels_dir / match_results.str(1));
					if (load_result) {
						return std::make_pair(Action::CLEAR_STACK, std::string{});
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("ledit usage:");
					GAME.console_output.emplace_back(".. file_name - open level editor with file");
				}
				return std::make_pair(Action::CONTINUE, std::string{});
			} else if (std::regex_match(command, std::regex{"medit(\\s.*)?"})) {

			} else if (std::regex_match(command, std::regex{"pedit(\\s.*)?"})) {
				if (std::smatch match_results; std::regex_match(command, match_results, std::regex{R"(pedit\s+([0-9]+)\s+([0-9]+)\s+(.+))"})) {
					auto x_offset = strtol(match_results.str(1).c_str(), nullptr, 0);
					auto y_offset = strtol(match_results.str(2).c_str(), nullptr, 0);
					auto load_result = GAME.load_pixel_editor(match_results.str(3), static_cast<int>(x_offset), static_cast<int>(y_offset));
					if (load_result) {
						return std::make_pair(Action::CLEAR_STACK, std::string{});
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("pedit usage:");
					GAME.console_output.emplace_back(".. x_offset y_offset file_name - open pixel editor with file");
				}
				return std::make_pair(Action::CONTINUE, std::string{});
			} else if (std::regex_match(command, std::regex{"sedit(\\s.*)?"})) {
				if (std::smatch match_results; std::regex_match(command, match_results, std::regex{R"(sedit(\s.*)?)"})) {
					auto trimmed_arg = string::trim(match_results.str(1));
					auto path = trimmed_arg.empty() ? (GAME.resource_dir / "SpriteSheets.json").string() : trimmed_arg;
					auto load_result = GAME.load_sheet_editor(path);
					if (load_result) {
						// Execute main menu the first time the sheet editor is run
						auto main_menu_result = State::create_execute_sync(&m2::ui::sheet_editor_main_menu);
						return std::make_pair(
								main_menu_result == Action::RETURN ? Action::CLEAR_STACK : main_menu_result,
								std::string{});
					}
					GAME.console_output.emplace_back(load_result.error());
				} else {
					GAME.console_output.emplace_back("sedit usage:");
					GAME.console_output.emplace_back(".. file_name - open sheet editor with file");
				}
				return std::make_pair(Action::CONTINUE, std::string{});
			} else if (std::regex_match(command, std::regex{"set(\\s.*)?"})) {
				if (std::smatch match_results; std::regex_match(command, match_results, std::regex{R"(set\s+([_a-zA-Z]+)\s+([a-zA-Z0-9]+))"})) {
					if (auto parameter = match_results.str(1); parameter == "game_height") {
						auto new_game_height = I(strtol(match_results.str(2).c_str(), nullptr, 0));
						GAME.recalculate_dimensions(GAME.dimensions().window.w, GAME.dimensions().window.h,
								new_game_height);
						return std::make_pair(Action::CONTINUE, std::string{});
					}
					GAME.console_output.emplace_back("Unknown parameter");
				} else {
					// TODO print help
				}
			} else if (command == "quit") {
				return std::make_pair(Action::QUIT, std::string{});
			} else if (command == "close") {
				return std::make_pair(Action::RETURN, std::string{});
			} else if (command.empty()) {
				// Do nothing
			} else {
				GAME.console_output.emplace_back("Available commands:");
				GAME.console_output.emplace_back("help - display this help");
				GAME.console_output.emplace_back("ledit - open level editor");
				GAME.console_output.emplace_back("medit - open midi editor");
				GAME.console_output.emplace_back("pedit - open pixel editor");
				GAME.console_output.emplace_back("sedit - open sheet editor");
				GAME.console_output.emplace_back("set - set game variable");
				GAME.console_output.emplace_back("close - close the console");
				GAME.console_output.emplace_back("quit - quit game");
			}

			return std::make_pair(Action::CONTINUE, std::string{});
		}
};

template<unsigned INDEX>
widget::TextBlueprint command_output_variant() {
	return {
			.initial_text = "",
			.alignment = TextAlignment::LEFT,
			.on_update = [](MAYBE const widget::Text &self) -> std::pair<Action, std::optional<std::string>> {
				return {Action::CONTINUE,
						INDEX < GAME.console_output.size() ? GAME.console_output[GAME.console_output.size() - INDEX - 1]
								: std::string()};
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
								.on_update = [](MAYBE const widget::Text &self) {
									return std::make_pair(Action::CONTINUE, LEVEL.message);
								}
						}
				}
		}
};
