#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/String.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/sdl/Detail.h>
#include <m2/sheet_editor/Ui.h>
#include <m2/ui/PanelBlueprint.h>
#include <m2/ui/Panel.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/Hidden.h>
#include <m2/ui/widget/Image.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/IntegerInput.h>
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
				auto main_menu_result = Panel::create_and_run_blocking(&m2::ui::sheet_editor_main_menu);
				return main_menu_result.is_return() ? make_clear_stack_action() : std::move(main_menu_result);
			}
			M2_GAME.console_output.emplace_back(load_result.error());
			return make_continue_action();
		} else if (command == "bsedit") {
			auto load_result = M2_GAME.load_bulk_sheet_editor();
			if (load_result) {
				// Execute main menu the first time the bulk sheet editor is run
				auto main_menu_result = Panel::create_and_run_blocking(&m2::ui::bulk_sheet_editor_main_menu);
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

Action Panel::run_blocking() {
	LOG_DEBUG("Executing UI");

	// Get a screenshot if background_texture is not already provided
	if (not _background_texture) {
		_background_texture = sdl::capture_screen_as_texture();
	}

	// Update initial contents
	if (auto return_value = update_contents(0.0f); not return_value.is_continue()) {
		LOG_DEBUG("Update action is not continue");
		return return_value;
	}

	Events events;
	auto last_loop_ticks = sdl::get_ticks();
	while (true) {
		auto current_ticks = sdl::get_ticks();
		auto delta_time_s = F(current_ticks - last_loop_ticks) / 1000.0f;
		last_loop_ticks = current_ticks;

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
				if (auto action = create_and_run_blocking(&console_ui); action.is_return()) {
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
				update_positions();
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
		if (auto return_value = update_contents(delta_time_s); not return_value.is_continue()) {
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

Panel::Panel(std::variant<const PanelBlueprint*, std::unique_ptr<PanelBlueprint>> static_or_unique_blueprint,
std::variant<std::monostate, RectI, RectF> fullscreen_or_pixel_rect_or_relation_to_game_and_hud,
	sdl::TextureUniquePtr background_texture)
	: _is_valid(true), _prev_text_input_state(SDL_IsTextInputActive()), _background_texture(std::move(background_texture)) {
	if (std::holds_alternative<const PanelBlueprint*>(static_or_unique_blueprint)) {
		// Static blueprint
		blueprint = std::get<const PanelBlueprint*>(static_or_unique_blueprint);
	} else {
		// Unique blueprint
		_owned_blueprint = std::move(std::get<std::unique_ptr<PanelBlueprint>>(static_or_unique_blueprint));
		blueprint = _owned_blueprint.get(); // Point `blueprint` to owned_blueprint
	}

	if (std::holds_alternative<std::monostate>(fullscreen_or_pixel_rect_or_relation_to_game_and_hud)) {
		// Fullscreen
		_relation_to_game_and_hud_dims = {0.0f, 0.0f, 1.0f, 1.0f};
	} else if (std::holds_alternative<RectI>(fullscreen_or_pixel_rect_or_relation_to_game_and_hud)) {
		// Pixel dims, convert to "relation to game_and_hud dimensions"
		const auto& pixel_rect = std::get<RectI>(fullscreen_or_pixel_rect_or_relation_to_game_and_hud);
		const auto& game_and_hud_dims = M2_GAME.dimensions().game_and_hud;
		_relation_to_game_and_hud_dims = RectF{
			F(pixel_rect.x - game_and_hud_dims.x) / F(game_and_hud_dims.w),
			F(pixel_rect.y - game_and_hud_dims.y) / F(game_and_hud_dims.h),
			F(pixel_rect.w) / F(game_and_hud_dims.w),
			F(pixel_rect.h) / F(game_and_hud_dims.h)};
	} else {
		// Relation to game_and_hud
		_relation_to_game_and_hud_dims = std::get<RectF>(fullscreen_or_pixel_rect_or_relation_to_game_and_hud);
	}

	// Previous text input state is saved. We can now disable it to start with a clean slate
	SDL_StopTextInput();

	// Create widgets
	for (const auto &widget_blueprint : blueprint->widgets) {
		widgets.push_back(create_widget_state(widget_blueprint));
		// Check if focus is requested
		if (widget_blueprint.initially_focused) {
			set_widget_focus_state(*widgets.back(), true);
		}
	}

	// Update initial positions
	update_positions();

	IF(blueprint->on_create)(*this);
}

Action Panel::create_and_run_blocking(
	std::variant<const PanelBlueprint*, std::unique_ptr<PanelBlueprint>> static_or_unique_blueprint,
	std::variant<std::monostate, RectI, RectF> fullscreen_or_pixel_rect_or_relation_to_game_and_hud,
	sdl::TextureUniquePtr background_texture) {
	// Check if there are other blocking UI panels
	if (M2_GAME.ui_begin_ticks) {
		// Execute panel without keeping time
		Panel panel{std::move(static_or_unique_blueprint), fullscreen_or_pixel_rect_or_relation_to_game_and_hud, std::move(background_texture)};
		return panel.run_blocking();
	} else {
		// Save begin ticks for later and other nested UIs
		M2_GAME.ui_begin_ticks = sdl::get_ticks();
		// Execute panel
		Panel panel{std::move(static_or_unique_blueprint), fullscreen_or_pixel_rect_or_relation_to_game_and_hud, std::move(background_texture)};
		auto action = panel.run_blocking();
		// Add pause ticks
		M2_GAME.add_pause_ticks(sdl::get_ticks_since(*M2_GAME.ui_begin_ticks));
		M2_GAME.ui_begin_ticks.reset();
		// Return
		return action;
	}
}

Panel::~Panel() {
	clear_focus();

	if (_prev_text_input_state) {
		SDL_StartTextInput();
	} else {
		SDL_StopTextInput();
	}
}

RectI Panel::rect_px() const {
	const auto& game_and_hud_dims = M2_GAME.dimensions().game_and_hud;
	return RectI{
		iround(F(game_and_hud_dims.x) + _relation_to_game_and_hud_dims.x * F(game_and_hud_dims.w)),
		iround(F(game_and_hud_dims.y) + _relation_to_game_and_hud_dims.y * F(game_and_hud_dims.h)),
		iround(_relation_to_game_and_hud_dims.w * F(game_and_hud_dims.w)),
		iround(_relation_to_game_and_hud_dims.h * F(game_and_hud_dims.h))};
}

void Panel::update_positions() {
	auto rect = rect_px();
	for (const auto &widget_state : widgets) {
		auto widget_rect = calculate_widget_rect(
		    rect, blueprint->w, blueprint->h, widget_state->blueprint->x, widget_state->blueprint->y,
		    widget_state->blueprint->w, widget_state->blueprint->h);
		widget_state->set_rect(widget_rect);
	}
}

Action Panel::handle_events(Events& events) {
	// Return if Panel not enabled
	if (!enabled) {
		return make_continue_action();
	}
	// Return if Panel is set to ignore events
	if (blueprint->ignore_events) {
		return make_continue_action();
	}

	// If the UI is cancellable, check if MENU button is pressed
	if (blueprint->cancellable && events.pop_key_press(Key::MENU)) {
		return make_return_action();
	}

	// First, deliver the event to the panel
	if (blueprint->on_event) {
		blueprint->on_event(*this, events);
	}
	// Then, deliver the event to the widgets
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
	auto rect = rect_px();
	events.clear_mouse_button_presses(rect);
	events.clear_mouse_button_releases(rect);
	events.clear_mouse_wheel_scrolls(rect);
	events.clear_mouse_button_down(rect);

	return make_continue_action();
}

Action Panel::update_contents(float delta_time_s) {
	// Return if Panel not enabled
	if (!enabled) {
		return make_continue_action();
	}

	// Check if timed out
	if (_timeout_s && *_timeout_s < 0.0f) {
		return make_return_action();
	}

	if (blueprint->on_update) {
		if (auto action = blueprint->on_update(*this); not action.is_continue()) {
			return action;
		}
	}
	for (const auto &widget : widgets | std::views::filter(is_widget_enabled)) {
		if (auto action = widget->on_update(); not action.is_continue()) {
			return action;
		} else {
			continue;
		}
	}

	// Decrement timeout
	if (_timeout_s) {
		*_timeout_s -= delta_time_s;
	}

	return make_continue_action();
}

void Panel::draw() {
	// Return if Panel not enabled
	if (!enabled) {
		return;
	}

	auto rect = rect_px();
	Widget::draw_rectangle(rect, blueprint->background_color);
	std::ranges::for_each(widgets | std::views::filter(is_widget_enabled), draw_widget);
	Widget::draw_border(rect, vertical_border_width_px(), horizontal_border_width_px());
}

int Panel::vertical_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		// Pixels per unit
		float pixel_pitch = F(rect_px().w) / F(blueprint->w);
		return std::max(1, iround(pixel_pitch * blueprint->border_width));
	}
}

int Panel::horizontal_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		// Pixels per unit
		float pixel_pitch = F(rect_px().h) / F(blueprint->h);
		return std::max(1, iround(pixel_pitch * blueprint->border_width));
	}
}

std::unique_ptr<Widget> Panel::create_widget_state(const WidgetBlueprint &widget_blueprint) {
	std::unique_ptr<Widget> state;

	using namespace m2::ui::widget;
	if (std::holds_alternative<HiddenBlueprint>(widget_blueprint.variant)) {
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

void Panel::set_widget_focus_state(Widget &w, const bool state) {
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

void Panel::clear_focus() {
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

m2::ui::Widget *m2::ui::find_text_widget(Panel &state, const std::string &text) {
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
	    .horizontal_alignment = TextHorizontalAlignment::LEFT,
	    .on_update = [](MAYBE widget::Text &self) {
			self.set_text(INDEX < M2_GAME.console_output.size()
				? M2_GAME.console_output[M2_GAME.console_output.size() - INDEX - 1]
				: std::string());
		    return make_continue_action();
	    }};
}

PanelBlueprint m2::ui::console_ui = {
    .w = 1,
    .h = 25,
    .border_width = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        WidgetBlueprint{
            .x = 0, .y = 0, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<23>()},
        WidgetBlueprint{
            .x = 0, .y = 1, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<22>()},
        WidgetBlueprint{
            .x = 0, .y = 2, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<21>()},
        WidgetBlueprint{
            .x = 0, .y = 3, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<20>()},
        WidgetBlueprint{
            .x = 0, .y = 4, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<19>()},
        WidgetBlueprint{
            .x = 0, .y = 5, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<18>()},
        WidgetBlueprint{
            .x = 0, .y = 6, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<17>()},
        WidgetBlueprint{
            .x = 0, .y = 7, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<16>()},
        WidgetBlueprint{
            .x = 0, .y = 8, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<15>()},
        WidgetBlueprint{
            .x = 0, .y = 9, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<14>()},
        WidgetBlueprint{
            .x = 0, .y = 10, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<13>()},
        WidgetBlueprint{
            .x = 0, .y = 11, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<12>()},
        WidgetBlueprint{
            .x = 0, .y = 12, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<11>()},
        WidgetBlueprint{
            .x = 0, .y = 13, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<10>()},
        WidgetBlueprint{
            .x = 0, .y = 14, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<9>()},
        WidgetBlueprint{
            .x = 0, .y = 15, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<8>()},
        WidgetBlueprint{
            .x = 0, .y = 16, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<7>()},
        WidgetBlueprint{
            .x = 0, .y = 17, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<6>()},
        WidgetBlueprint{
            .x = 0, .y = 18, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<5>()},
        WidgetBlueprint{
            .x = 0, .y = 19, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<4>()},
        WidgetBlueprint{
            .x = 0, .y = 20, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<3>()},
        WidgetBlueprint{
            .x = 0, .y = 21, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<2>()},
        WidgetBlueprint{
            .x = 0, .y = 22, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<1>()},
        WidgetBlueprint{
            .x = 0, .y = 23, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<0>()},
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

const PanelBlueprint m2::ui::message_box_ui = {
    .w = 1,
    .h = 1,
    .border_width = 0,
	.ignore_events = true,
    .widgets = {WidgetBlueprint{
        .initially_enabled = false,
        .x = 0,
        .y = 0,
        .w = 1,
        .h = 1,
        .border_width = 0,
        .background_color = SDL_Color{127, 127, 127, 127},
        .variant =
            widget::TextBlueprint{
				.horizontal_alignment = TextHorizontalAlignment::LEFT,
				.on_update = [](MAYBE widget::Text &self) {
					if (M2_LEVEL.message) {
						self.set_text(*M2_LEVEL.message);
					}
					return make_continue_action();
				}
		}
	}}
};
