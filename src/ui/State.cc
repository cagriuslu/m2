#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/String.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/sdl/Detail.h>
#include <m2/sheet_editor/Ui.h>
#include <m2/ui/Blueprint.h>
#include <m2/ui/State.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/Hidden.h>
#include <m2/ui/widget/Image.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/IntegerInput.h>
#include <m2/ui/widget/NestedUi.h>
#include <m2/ui/widget/ProgressBar.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextSelection.h>

#include <ranges>
#include <regex>

using namespace m2;
using namespace m2::ui;

namespace {
	// Filters
	constexpr auto is_widget_enabled = [](const auto &w) { return w->enabled; };
	constexpr auto is_widget_focused = [](const auto &w) { return w->focused; };
	// Actions
	constexpr auto draw_widget = [](const auto &w) { w->on_draw(); };

	Action handle_console_command(const std::string &command) {
		M2_GAME.console_output.emplace_back(">> " + command);

		if (std::regex_match(command, std::regex{"ledit(\\s.*)?"})) {
			if (std::smatch match_results; std::regex_match(command, match_results, std::regex{"ledit\\s+(.+)"})) {
				auto load_result = M2_GAME.load_level_editor(M2_GAME.levels_dir / match_results.str(1));
				if (load_result) {
					return make_clear_stack_action();
				}
				M2_GAME.console_output.emplace_back(load_result.error());
			} else {
				M2_GAME.console_output.emplace_back("ledit usage:");
				M2_GAME.console_output.emplace_back(".. file_name - open level editor with file");
			}
			return make_continue_action();
		} else if (std::regex_match(command, std::regex{"medit(\\s.*)?"})) {
			// MIDI editor (?)
		} else if (std::regex_match(command, std::regex{"pedit(\\s.*)?"})) {
			if (std::smatch match_results;
			    std::regex_match(command, match_results, std::regex{R"(pedit\s+([0-9]+)\s+([0-9]+)\s+(.+))"})) {
				auto x_offset = strtol(match_results.str(1).c_str(), nullptr, 0);
				auto y_offset = strtol(match_results.str(2).c_str(), nullptr, 0);
				auto load_result = M2_GAME.load_pixel_editor(
				    match_results.str(3), static_cast<int>(x_offset), static_cast<int>(y_offset));
				if (load_result) {
					return make_clear_stack_action();
				}
				M2_GAME.console_output.emplace_back(load_result.error());
			} else {
				M2_GAME.console_output.emplace_back("pedit usage:");
				M2_GAME.console_output.emplace_back(".. x_offset y_offset file_name - open pixel editor with file");
			}
			return make_continue_action();
		} else if (command == "sedit") {
			auto load_result = M2_GAME.load_sheet_editor();
			if (load_result) {
				// Execute main menu the first time the sheet editor is run
				auto main_menu_result = State::create_execute_sync(&m2::ui::sheet_editor_main_menu);
				return main_menu_result.is_return() ? make_clear_stack_action() : std::move(main_menu_result);
			}
			M2_GAME.console_output.emplace_back(load_result.error());
			return make_continue_action();
		} else if (command == "bsedit") {
			auto load_result = M2_GAME.load_bulk_sheet_editor();
			if (load_result) {
				// Execute main menu the first time the bulk sheet editor is run
				auto main_menu_result = State::create_execute_sync(&m2::ui::bulk_sheet_editor_main_menu);
				return main_menu_result.is_return() ? make_clear_stack_action() : std::move(main_menu_result);
			}
			M2_GAME.console_output.emplace_back(load_result.error());
			return make_continue_action();
		} else if (std::regex_match(command, std::regex{"set(\\s.*)?"})) {
			if (std::smatch match_results;
			    std::regex_match(command, match_results, std::regex{R"(set\s+([_a-zA-Z]+)\s+([a-zA-Z0-9]+))"})) {
				if (auto parameter = match_results.str(1); parameter == "game_height") {
					auto new_game_height = I(strtol(match_results.str(2).c_str(), nullptr, 0));
					M2_GAME.recalculate_dimensions(
					    M2_GAME.dimensions().window.w, M2_GAME.dimensions().window.h, new_game_height);
					return make_continue_action();
				}
				M2_GAME.console_output.emplace_back("Unknown parameter");
			} else {
				// TODO print help
			}
		} else if (command == "quit") {
			return make_quit_action();
		} else if (command == "close") {
			return make_return_action();
		} else if (command.empty()) {
			// Do nothing
		} else {
			M2_GAME.console_output.emplace_back("Available commands:");
			M2_GAME.console_output.emplace_back("help - display this help");
			M2_GAME.console_output.emplace_back("ledit - open level editor");
			M2_GAME.console_output.emplace_back("medit - open midi editor");
			M2_GAME.console_output.emplace_back("pedit - open pixel editor");
			M2_GAME.console_output.emplace_back("sedit - open sheet editor");
			M2_GAME.console_output.emplace_back("set - set game variable");
			M2_GAME.console_output.emplace_back("close - close the console");
			M2_GAME.console_output.emplace_back("quit - quit game");
		}
		return make_continue_action();
	}
}  // namespace

State::State(std::variant<const Blueprint*, std::unique_ptr<Blueprint>> bp, sdl::TextureUniquePtr background_texture)
	: _prev_text_input_state(SDL_IsTextInputActive()), _background_texture(std::move(background_texture)) {
	if (std::holds_alternative<const Blueprint*>(bp)) {
		blueprint = std::get<const Blueprint*>(bp);
	} else {
		_managed_blueprint = std::move(std::get<std::unique_ptr<Blueprint>>(bp));
		blueprint = _managed_blueprint.get();
	}

	// Previous text input state is saved, not disable it to start with a clean slate
	SDL_StopTextInput();

	for (const auto &widget_blueprint : blueprint->widgets) {
		// Create widget
		widgets.push_back(create_widget_state(widget_blueprint));
		// Check if focus is requested
		if (widget_blueprint.initially_focused) {
			set_widget_focus_state(*widgets.back(), true);
		}
	}
}

Action State::create_execute_sync(std::variant<const Blueprint*, std::unique_ptr<Blueprint>> blueprint) {
	return create_execute_sync(std::move(blueprint), M2_GAME.dimensions().window);
}

Action State::create_execute_sync(std::variant<const Blueprint*, std::unique_ptr<Blueprint>> blueprint, const RectI rect, sdl::TextureUniquePtr background_texture) {
	// Check if there are other blocking UIs
	if (M2_GAME.ui_begin_ticks) {
		// Execute state without keeping time
		State state{std::move(blueprint)};
		return state.execute(rect);
	} else {
		// Save begin ticks for later and other nested UIs
		M2_GAME.ui_begin_ticks = sdl::get_ticks();
		// Execute state
		State state{std::move(blueprint), std::move(background_texture)};
		auto action = state.execute(rect);
		// Add pause ticks
		M2_GAME.add_pause_ticks(sdl::get_ticks_since(*M2_GAME.ui_begin_ticks));
		M2_GAME.ui_begin_ticks.reset();
		return action;
	}
}

State::~State() {
	clear_focus();

	if (_prev_text_input_state) {
		SDL_StartTextInput();
	} else {
		SDL_StopTextInput();
	}
}

Action State::execute(const RectI rect) {
	LOG_DEBUG("Executing UI");

	// Save relation to window, use in case of resize
	const auto &winrect = M2_GAME.dimensions().window;
	const auto relation_to_window = RectF{
	    static_cast<float>(rect.x - winrect.x) / static_cast<float>(winrect.w),
	    static_cast<float>(rect.y - winrect.y) / static_cast<float>(winrect.h),
	    static_cast<float>(rect.w) / static_cast<float>(winrect.w),
	    static_cast<float>(rect.h) / static_cast<float>(winrect.h)};

	// Get a screenshot if background_texture is not already provided
	if (not _background_texture) {
		_background_texture = sdl::capture_screen_as_texture();
	}

	// Update initial positions
	update_positions(rect);

	// Update initial contents
	if (auto return_value = update_contents(); not return_value.is_continue()) {
		LOG_DEBUG("Update action is not continue");
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
				return make_quit_action();
			}

			// Handle console action
			if ((not console_command.empty() || events.pop_key_press(Key::CONSOLE)) &&
			    blueprint != &console_ui) {  // Do not open console on top of console

				// Initialize console with command
				std::get<widget::TextInputBlueprint>(console_ui.widgets[24].variant).initial_text = console_command;
				console_command.clear();

				LOG_INFO("Opening console");
				if (auto action = create_execute_sync(&console_ui); action.is_return()) {
					// Continue with the prev UI
					LOG_DEBUG("Console returned");
				} else if (action.is_clear_stack() || action.is_quit()) {
					LOG_DEBUG("Console clear stack or quit");
					return action;
				}
			}

			// Handle resize action
			if (const auto window_resize = events.pop_window_resize(); window_resize) {
				M2_GAME.recalculate_dimensions(window_resize->x, window_resize->y);
				update_positions(RectI{
				    static_cast<int>(
				        round(static_cast<float>(winrect.x) + relation_to_window.x * static_cast<float>(winrect.w))),
				    static_cast<int>(
				        round(static_cast<float>(winrect.y) + relation_to_window.y * static_cast<float>(winrect.h))),
				    static_cast<int>(round(relation_to_window.w * static_cast<float>(winrect.w))),
				    static_cast<int>(round(relation_to_window.h * static_cast<float>(winrect.h)))});
			}

			// Handle events
			if (auto return_value = handle_events(events); not return_value.is_continue()) {
				return return_value;
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Update contents
		if (auto return_value = update_contents(); not return_value.is_continue()) {
			return return_value;
		}

		// Clear screen
		SDL_SetRenderDrawColor(M2_GAME.renderer, 0, 0, 0, 255);
		SDL_RenderClear(M2_GAME.renderer);
		SDL_RenderCopy(M2_GAME.renderer, _background_texture.get(), nullptr, nullptr);

		// Draw UI elements
		draw();

		// Present
		SDL_RenderPresent(M2_GAME.renderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
	}
}

void State::update_positions(const RectI &rect) {
	this->rect_px = rect;
	for (const auto &widget_state : widgets) {
		auto widget_rect = calculate_widget_rect(
		    rect, blueprint->w, blueprint->h, widget_state->blueprint->x, widget_state->blueprint->y,
		    widget_state->blueprint->w, widget_state->blueprint->h);
		widget_state->on_position_update(widget_rect);
	}
}

Action State::handle_events(Events& events) {
	// Return if State not enabled
	if (!enabled) {
		return make_continue_action();
	}

	// If the UI is cancellable, check if MENU button is pressed
	if (blueprint->cancellable && events.pop_key_press(Key::MENU)) {
		return make_return_action();
	}

	for (auto &widget : widgets | std::views::filter(is_widget_enabled)) {
		if (auto action = widget->on_event(events); action.is_continue()) {
			action.if_continue_with_focus_state([&](bool focus_state) {
				set_widget_focus_state(*widget, focus_state);
			});
			continue;
		} else {
			return action;
		}
	}

	// Clear mouse events if it's inside the UI rect
	events.clear_mouse_button_presses(rect_px);
	events.clear_mouse_button_releases(rect_px);
	events.clear_mouse_wheel_scrolls(rect_px);
	events.clear_mouse_button_down(rect_px);

	return make_continue_action();
}

Action State::update_contents() {
	// Return if State not enabled
	if (!enabled) {
		return make_continue_action();
	}

	for (const auto &widget : widgets | std::views::filter(is_widget_enabled)) {
		if (auto action = widget->on_update(); not action.is_continue()) {
			return action;
		} else {
			continue;
		}
	}
	return make_continue_action();
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
	} else if (std::holds_alternative<IntegerInputBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<IntegerInput>(this, &widget_blueprint);
	} else if (std::holds_alternative<CheckboxWithTextBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<CheckboxWithText>(this, &widget_blueprint);
	} else {
		throw M2_ERROR("Implementation");
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
	    widgets | std::views::filter(is_widget_focused), [&](const auto &it) { set_widget_focus_state(*it, false); });
}

RectI ui::calculate_widget_rect(
    const RectI &root_rect_px, const unsigned root_w, const unsigned root_h, const int child_x, const int child_y,
    const unsigned child_w, const unsigned child_h) {
	const auto pixels_per_unit_w = static_cast<float>(root_rect_px.w) / static_cast<float>(root_w);
	const auto pixels_per_unit_h = static_cast<float>(root_rect_px.h) / static_cast<float>(root_h);
	return RectI{
	    root_rect_px.x + static_cast<int>(roundf(static_cast<float>(child_x) * pixels_per_unit_w)),
	    root_rect_px.y + static_cast<int>(roundf(static_cast<float>(child_y) * pixels_per_unit_h)),
	    static_cast<int>(roundf(static_cast<float>(child_w) * pixels_per_unit_w)),
	    static_cast<int>(roundf(static_cast<float>(child_h) * pixels_per_unit_h))};
}

m2::ui::Widget *m2::ui::find_text_widget(State &state, const std::string &text) {
	const auto it = std::ranges::find_if(
	    state.widgets,
	    // Predicate
	    [=](const auto *blueprint) {
		    const auto *text_variant = std::get_if<ui::widget::TextBlueprint>(&blueprint->variant);
		    // If widget is Text and the button is labelled correctly
		    return text_variant && text_variant->text == text;
	    },
	    // Projection
	    [](const auto &unique_blueprint) { return unique_blueprint->blueprint; });

	return (it != state.widgets.end()) ? it->get() : nullptr;
}

template <unsigned INDEX>
widget::TextBlueprint command_output_variant() {
	return {
	    .text = "",
	    .alignment = TextAlignment::LEFT,
	    .on_update = [](MAYBE widget::Text &self) {
			self.set_text(INDEX < M2_GAME.console_output.size()
				? M2_GAME.console_output[M2_GAME.console_output.size() - INDEX - 1]
				: std::string());
		    return make_continue_action();
	    }};
}

Blueprint m2::ui::console_ui = {
    .w = 1,
    .h = 25,
    .border_width_px = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        WidgetBlueprint{
            .x = 0, .y = 0, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<23>()},
        WidgetBlueprint{
            .x = 0, .y = 1, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<22>()},
        WidgetBlueprint{
            .x = 0, .y = 2, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<21>()},
        WidgetBlueprint{
            .x = 0, .y = 3, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<20>()},
        WidgetBlueprint{
            .x = 0, .y = 4, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<19>()},
        WidgetBlueprint{
            .x = 0, .y = 5, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<18>()},
        WidgetBlueprint{
            .x = 0, .y = 6, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<17>()},
        WidgetBlueprint{
            .x = 0, .y = 7, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<16>()},
        WidgetBlueprint{
            .x = 0, .y = 8, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<15>()},
        WidgetBlueprint{
            .x = 0, .y = 9, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<14>()},
        WidgetBlueprint{
            .x = 0, .y = 10, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<13>()},
        WidgetBlueprint{
            .x = 0, .y = 11, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<12>()},
        WidgetBlueprint{
            .x = 0, .y = 12, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<11>()},
        WidgetBlueprint{
            .x = 0, .y = 13, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<10>()},
        WidgetBlueprint{
            .x = 0, .y = 14, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<9>()},
        WidgetBlueprint{
            .x = 0, .y = 15, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<8>()},
        WidgetBlueprint{
            .x = 0, .y = 16, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<7>()},
        WidgetBlueprint{
            .x = 0, .y = 17, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<6>()},
        WidgetBlueprint{
            .x = 0, .y = 18, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<5>()},
        WidgetBlueprint{
            .x = 0, .y = 19, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<4>()},
        WidgetBlueprint{
            .x = 0, .y = 20, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<3>()},
        WidgetBlueprint{
            .x = 0, .y = 21, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<2>()},
        WidgetBlueprint{
            .x = 0, .y = 22, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<1>()},
        WidgetBlueprint{
            .x = 0, .y = 23, .w = 1, .h = 1, .border_width_px = 0, .variant = command_output_variant<0>()},
        WidgetBlueprint{
            .initially_focused = true,
            .x = 0,
            .y = 24,
            .w = 1,
            .h = 1,
            .background_color = SDL_Color{27, 27, 27, 255},
            .variant = widget::TextInputBlueprint{
                .initial_text = "", // May be overriden with console_command on startup
                .on_action = [](const widget::TextInput &self) -> std::pair<Action, std::optional<std::string>> {
	                const auto &command = self.text_input();
	                return std::make_pair(handle_console_command(command), std::string{});
                }}}}};

const Blueprint m2::ui::message_box_ui = {
    .w = 1,
    .h = 1,
    .border_width_px = 0,
    .widgets = {WidgetBlueprint{
        .initially_enabled = false,
        .x = 0,
        .y = 0,
        .w = 1,
        .h = 1,
        .border_width_px = 0,
        .background_color = SDL_Color{127, 127, 127, 127},
        .variant =
            widget::TextBlueprint{
				.alignment = TextAlignment::LEFT,
				.on_update = [](MAYBE widget::Text &self) {
					if (M2_LEVEL.message) {
						self.set_text(*M2_LEVEL.message);
					}
					return make_continue_action();
				}
		}
	}}
};
