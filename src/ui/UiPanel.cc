#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/String.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/sdl/Detail.h>
#include <m2/sheet_editor/Ui.h>
#include <m2/ui/UiPanelBlueprint.h>
#include <m2/ui/UiPanel.h>
#include <m2/ui/UiWidgetBlueprint.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/Hidden.h>
#include <m2/ui/widget/Image.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/ui/widget/ProgressBar.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextSelection.h>

#include <ranges>
#include <regex>

using namespace m2;
using namespace m2;

namespace {
	// Filters
	constexpr auto is_widget_enabled = [](const auto &w) { return w->enabled; };
	constexpr auto IsWidgetFocused = [](const auto &w) { return w->IsFocused(); };
	// Actions
	constexpr auto draw_widget = [](const auto &w) { w->Draw(); };

	UiAction handle_console_command(const std::string &command) {
		M2_GAME.console_output.emplace_back(">> " + command);

		if (std::regex_match(command, std::regex{"ledit(\\s.*)?"})) {
			if (std::smatch match_results; std::regex_match(command, match_results, std::regex{"ledit\\s+(.+)"})) {
				auto load_result = M2_GAME.LoadLevelEditor((M2_GAME.GetResources().GetLevelsDir() / match_results.str(1)).string());
				if (load_result) {
					return MakeClearStackAction();
				}
				M2_GAME.console_output.emplace_back(load_result.error());
			} else {
				M2_GAME.console_output.emplace_back("ledit usage:");
				M2_GAME.console_output.emplace_back(".. file_name - open level editor with file");
			}
			return MakeContinueAction();
		} else if (std::regex_match(command, std::regex{"medit(\\s.*)?"})) {
			// MIDI editor (?)
		} else if (command == "sedit") {
			auto load_result = M2_GAME.LoadSheetEditor();
			if (load_result) {
				// Execute main menu the first time the sheet editor is run
				auto main_menu_result = UiPanel::create_and_run_blocking(&m2::sheet_editor_main_menu);
				return main_menu_result.IsReturn() ? MakeClearStackAction() : std::move(main_menu_result);
			}
			M2_GAME.console_output.emplace_back(load_result.error());
			return MakeContinueAction();
		} else if (command == "bsedit") {
			auto load_result = M2_GAME.LoadBulkSheetEditor();
			if (load_result) {
				// Execute main menu the first time the bulk sheet editor is run
				auto main_menu_result = UiPanel::create_and_run_blocking(&m2::bulk_sheet_editor::gMainMenu);
				return main_menu_result.IsReturn() ? MakeClearStackAction() : std::move(main_menu_result);
			}
			M2_GAME.console_output.emplace_back(load_result.error());
			return MakeContinueAction();
		} else if (std::regex_match(command, std::regex{"mvbg(\\s.*)?"})) {
			// Move background
			if (std::smatch match_results; std::regex_match(command, match_results, std::regex{R"(mvbg\s+([0-9]+)\s+([0-9]+)\s+(.+))"})) {
				auto layer_from = strtol(match_results.str(1).c_str(), nullptr, 0);
				auto layer_to = strtol(match_results.str(2).c_str(), nullptr, 0);
				auto level = match_results.str(3);
				if (auto success = MoveBackground(I(layer_from), I(layer_to), level); not success) {
					M2_GAME.console_output.emplace_back(success.error());
				}
			} else {
				M2_GAME.console_output.emplace_back("mvbg usage:");
				M2_GAME.console_output.emplace_back(".. from to - move one background layer into another");
			}
			return MakeContinueAction();
		} else if (command == "quit") {
			return MakeQuitAction();
		} else if (command == "close") {
			return MakeReturnAction();
		} else if (command.empty()) {
			// Do nothing
		} else {
			M2_GAME.console_output.emplace_back("Available commands:");
			M2_GAME.console_output.emplace_back("help - display this help");
			M2_GAME.console_output.emplace_back("ledit - open level editor");
			M2_GAME.console_output.emplace_back("medit - open midi editor");
			M2_GAME.console_output.emplace_back("sedit - open sheet editor");
			M2_GAME.console_output.emplace_back("bsedit - open bulk sheet editor");
			M2_GAME.console_output.emplace_back("mvbg - move background");
			M2_GAME.console_output.emplace_back("close - close the console");
			M2_GAME.console_output.emplace_back("quit - quit game");
		}
		return MakeContinueAction();
	}
}  // namespace

UiAction UiPanel::run_blocking() {
	LOG_DEBUG("Running blocking UI", blueprint->name);

	if (M2_GAME.IsMultiPlayer() && not M2_GAME.IsOurTurn()) {
		// Running a blocking UI in this case could lead to client commands being blocked from processing
		LOG_WARN("Running blocking UI panel during a multiplayer game while it's not our turn");
	}

	// Get a screenshot if background_texture is not already provided
	if (not _background_texture) {
		_background_texture = sdl::capture_screen_as_texture();
	}

	// Update initial contents
	if (auto return_value = UpdateContents(0.0f); not return_value.IsContinue()) {
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
		events.Clear();
		if (events.Gather()) {
			// Handle quit action
			if (events.PopQuit()) {
				return MakeQuitAction();
			}

			// Handle console action
			if ((not console_command.empty() || events.PopKeyPress(m2g::pb::KeyType::CONSOLE)) &&
				blueprint != &console_ui) {  // Do not open console on top of console

				// Initialize console with command
				std::get<widget::TextInputBlueprint>(console_ui.widgets[24].variant).initial_text = console_command;
				console_command.clear();

				LOG_INFO("Opening console");
				if (auto action = create_and_run_blocking(&console_ui); action.IsReturn()) {
					// Continue with the prev UI
					LOG_DEBUG("Console returned");
				} else if (action.IsClearStack() || action.IsQuit()) {
					LOG_DEBUG("Console clear stack or quit");
					return action;
				}
			}

			// Handle resize action
			if (const auto window_resize = events.PopWindowResize(); window_resize) {
				M2_GAME.OnWindowResize();
				// TODO what about the other sync panels in the stack?
				RecalculateRects();
			}

			// Handle events
			if (auto return_value = HandleEvents(events); not return_value.IsContinue()) {
				return return_value;
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Update contents
		if (auto return_value = UpdateContents(delta_time_s); not return_value.IsContinue()) {
			return return_value;
		}

		// Clear screen
		SDL_SetRenderDrawColor(M2_GAME.renderer, 0, 0, 0, 255);
		SDL_RenderClear(M2_GAME.renderer);
		SDL_RenderCopy(M2_GAME.renderer, _background_texture.get(), nullptr, nullptr);

		// Draw UI elements
		Draw();
		M2_GAME.DrawEnvelopes();

		// Present
		SDL_RenderPresent(M2_GAME.renderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
	}
}

UiPanel::UiPanel(std::variant<const UiPanelBlueprint*, std::unique_ptr<UiPanelBlueprint>> static_or_unique_blueprint,
		const std::variant<std::monostate, RectI, RectF>& fullscreen_or_pixel_rect_or_relation_to_game_and_hud,
		sdl::TextureUniquePtr background_texture)
		: _prev_text_input_state(SDL_IsTextInputActive()), _background_texture(std::move(background_texture)) {
	if (std::holds_alternative<const UiPanelBlueprint*>(static_or_unique_blueprint)) {
		// Static blueprint
		blueprint = std::get<const UiPanelBlueprint*>(static_or_unique_blueprint);
	} else {
		// Unique blueprint
		_owned_blueprint = std::move(std::get<std::unique_ptr<UiPanelBlueprint>>(static_or_unique_blueprint));
		blueprint = _owned_blueprint.get(); // Point `blueprint` to owned_blueprint
	}

	LOG_DEBUG("Initializing UI", blueprint->name);

	if (std::holds_alternative<std::monostate>(fullscreen_or_pixel_rect_or_relation_to_game_and_hud)) {
		// Fullscreen
		_relation_to_game_and_hud_dims = {0.0f, 0.0f, 1.0f, 1.0f};
	} else if (std::holds_alternative<RectI>(fullscreen_or_pixel_rect_or_relation_to_game_and_hud)) {
		// Pixel dims, convert to "relation to game_and_hud dimensions"
		const auto& pixel_rect = std::get<RectI>(fullscreen_or_pixel_rect_or_relation_to_game_and_hud);
		const auto& game_and_hud_dims = M2_GAME.Dimensions().GameAndHud();
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

	// Set timeout if necessary
	if (blueprint->timeout_s != 0.0f) {
		_timeout_s = blueprint->timeout_s;
	}

	// Create widgets
	for (const auto &widget_blueprint : blueprint->widgets) {
		widgets.emplace_back(create_widget_state(widget_blueprint));
		// Check if focus is requested
		if (widget_blueprint.initially_focused) {
			set_widget_focus_state(*widgets.back(), true);
		}
	}

	// Update initial positions
	RecalculateRects();

	IF(blueprint->onCreate)(*this);
}

UiAction UiPanel::create_and_run_blocking(std::variant<const UiPanelBlueprint*,
		std::unique_ptr<UiPanelBlueprint>> static_or_unique_blueprint,
		const std::variant<std::monostate, RectI, RectF>& fullscreen_or_pixel_rect_or_relation_to_game_and_hud,
		sdl::TextureUniquePtr background_texture) {
	// Check if there is already a blocking UI panel
	if (not M2_GAME.HasLevel() || M2_LEVEL.rootBlockingUiBeginTicks) {
		// Execute panel without keeping time, it's being kept already
		return UiPanel{std::move(static_or_unique_blueprint), fullscreen_or_pixel_rect_or_relation_to_game_and_hud,
				std::move(background_texture)}.run_blocking();
	}

	// Save begin ticks for later and other nested UIs
	M2_LEVEL.rootBlockingUiBeginTicks = sdl::get_ticks();
	// Execute panel
	auto action = UiPanel{std::move(static_or_unique_blueprint),
			fullscreen_or_pixel_rect_or_relation_to_game_and_hud, std::move(background_texture)}.run_blocking();
	// Add pause ticks
	M2_LEVEL.AddPauseTicks(sdl::get_ticks_since(*M2_LEVEL.rootBlockingUiBeginTicks));
	M2_LEVEL.rootBlockingUiBeginTicks.reset();
	// Return
	return action;
}

UiPanel::~UiPanel() {
	if (blueprint && blueprint->onDestroy) {
		blueprint->onDestroy();
	}

	clear_focus();

	if (_prev_text_input_state) {
		SDL_StartTextInput();
	} else {
		SDL_StopTextInput();
	}
}

const std::string& UiPanel::Name() const {
	return blueprint->name;
}
bool UiPanel::IsKilled() const {
	return static_cast<bool>(_returnValueContainer);
}
const AnyReturnContainer* UiPanel::PeekReturnValueContainer() const {
	return IsKilled() ? &*_returnValueContainer : nullptr;
}
RectI UiPanel::Rect() const {
	const auto& game_and_hud_dims = M2_GAME.Dimensions().GameAndHud();
	return RectI{
		RoundI(F(game_and_hud_dims.x) + _relation_to_game_and_hud_dims.x * F(game_and_hud_dims.w)),
		RoundI(F(game_and_hud_dims.y) + _relation_to_game_and_hud_dims.y * F(game_and_hud_dims.h)),
		RoundI(_relation_to_game_and_hud_dims.w * F(game_and_hud_dims.w)),
		RoundI(_relation_to_game_and_hud_dims.h * F(game_and_hud_dims.h))};
}

void UiPanel::KillWithReturnValue(AnyReturnContainer&& arc) {
	// Destruct self
	this->~UiPanel();
	// Re-construct self in-place with return value
	new (this) UiPanel(std::move(arc));
}

void UiPanel::SetTopLeftPosition(const VecI& newPosition) {
	const auto& game_and_hud_dims = M2_GAME.Dimensions().GameAndHud();
	_relation_to_game_and_hud_dims = RectF{
			F(newPosition.x) / F(game_and_hud_dims.w),
			F(newPosition.y) / F(game_and_hud_dims.h),
			_relation_to_game_and_hud_dims.w,
			_relation_to_game_and_hud_dims.h};
	RecalculateRects();
}
void UiPanel::SetTimeout(const float timeoutS) {
	if (timeoutS < 0.0f) {
		throw M2_ERROR("Given timeout is negative: " + ToString(timeoutS));
	}
	_timeout_s = timeoutS;
}
void UiPanel::ClearTimeout() {
	_timeout_s.reset();
}

void UiPanel::RecalculateRects() {
	auto rect = Rect();
	for (const auto &widget_state : widgets) {
		auto widget_rect = CalculateWidgetRect(
		    rect, blueprint->w, blueprint->h, widget_state->blueprint->x, widget_state->blueprint->y,
		    widget_state->blueprint->w, widget_state->blueprint->h);
		widget_state->SetRect(widget_rect);
	}
}

UiAction UiPanel::HandleEvents(Events& events, bool IsPanning) {
	// Return if UiPanel not enabled
	if (IsKilled() || not enabled) {
		return MakeContinueAction();
	}
	// Return if UiPanel is set to ignore events
	if (blueprint->ignore_events) {
		return MakeContinueAction();
	}

	// If the UI is cancellable, check if MENU button is pressed
	if (blueprint->cancellable && events.PopKeyPress(m2g::pb::KeyType::PAUSE)) {
		return MakeReturnAction();
	}

	std::optional<UiAction> action;

	// First, deliver the event to the panel
	if (blueprint->onEvent) {
		action = blueprint->onEvent(*this, events);
	}

	if (not action || action->IsContinue()) {
		// Then, deliver the event to the widgets
		for (auto &widget : widgets | std::views::filter(is_widget_enabled)) {
			action = widget->HandleEvents(events);
			action->IfContinueWithFocusState([&](const bool focus_state) { set_widget_focus_state(*widget, focus_state); });
			if (not action->IsContinue()) {
				break;
			}
		}
	}

	// Clear mouse events if the mouse is inside the UI element so that it isn't delivered to game objects. This
	// behavior can partially be overwritten with Game::EnablePanning().
	const auto rect = Rect();
	events.ClearMouseButtonPresses(rect);
	events.ClearMouseButtonReleases(rect);
	events.ClearMouseWheelScrolls(rect);
	if (not IsPanning) {
		events.ClearMouseButtonDown(rect);
	}

	if (not action) {
		return MakeContinueAction();
	}
	return std::move(*action);
}

UiAction UiPanel::UpdateContents(float delta_time_s) {
	// Return if UiPanel not enabled
	if (IsKilled() || not enabled) {
		return MakeContinueAction();
	}

	// Check if timed out
	if (_timeout_s && *_timeout_s < 0.0f) {
		enabled = false;
		return MakeContinueAction();
	}

	if (blueprint->onUpdate) {
		if (auto action = blueprint->onUpdate(*this); not action.IsContinue()) {
			return action;
		}
	}
	for (const auto &widget : widgets | std::views::filter(is_widget_enabled)) {
		if (auto action = widget->UpdateContents(); not action.IsContinue()) {
			return action;
		}
	}

	// Decrement timeout
	if (_timeout_s) {
		*_timeout_s -= delta_time_s;
	}

	return MakeContinueAction();
}

void UiPanel::Draw() {
	if (IsKilled() || not enabled) {
		return;
	}

	auto rect = Rect();
	UiWidget::draw_rectangle(rect, blueprint->background_color);
	std::ranges::for_each(widgets | std::views::filter(is_widget_enabled), draw_widget);
	UiWidget::draw_border(rect, vertical_border_width_px(), horizontal_border_width_px());
}

int UiPanel::vertical_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		// Pixels per unit
		float pixel_pitch = F(Rect().w) / F(blueprint->w);
		return std::max(1, RoundI(pixel_pitch * blueprint->border_width));
	}
}

int UiPanel::horizontal_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		// Pixels per unit
		float pixel_pitch = F(Rect().h) / F(blueprint->h);
		return std::max(1, RoundI(pixel_pitch * blueprint->border_width));
	}
}

std::unique_ptr<UiWidget> UiPanel::create_widget_state(const UiWidgetBlueprint &widget_blueprint) {
	std::unique_ptr<UiWidget> state;

	using namespace m2::widget;
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
	} else if (std::holds_alternative<IntegerSelectionBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<IntegerSelection>(this, &widget_blueprint);
	} else if (std::holds_alternative<CheckboxWithTextBlueprint>(widget_blueprint.variant)) {
		state = std::make_unique<CheckboxWithText>(this, &widget_blueprint);
	} else {
		throw M2_ERROR("Implementation");
	}

	return state;
}

void UiPanel::set_widget_focus_state(UiWidget &w, const bool state) {
	// Clear the other focused widget
	if (state && not w.IsFocused()) {
		clear_focus();
	}
	w.SetFocusState(state);
}

void UiPanel::clear_focus() {
	// Check if there's an already focused widget
	std::ranges::for_each(
	    widgets | std::views::filter(IsWidgetFocused), [&](const auto &it) { set_widget_focus_state(*it, false); });
}

RectI m2::CalculateWidgetRect(
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

m2::UiWidget *m2::FindTextWidget(UiPanel &state, const std::string &text) {
	const auto it = std::ranges::find_if(
	    state.widgets,
	    // Predicate
	    [=](const auto *blueprint) {
		    const auto *text_variant = std::get_if<widget::TextBlueprint>(&blueprint->variant);
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
	    .onUpdate = [](MAYBE widget::Text &self) {
			self.set_text(INDEX < M2_GAME.console_output.size()
				? M2_GAME.console_output[M2_GAME.console_output.size() - INDEX - 1]
				: std::string());
		    return MakeContinueAction();
	    }};
}

UiPanelBlueprint m2::console_ui = {
	.name = "Console",
    .w = 1,
    .h = 25,
    .border_width = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        UiWidgetBlueprint{
            .x = 0, .y = 0, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<23>()},
        UiWidgetBlueprint{
            .x = 0, .y = 1, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<22>()},
        UiWidgetBlueprint{
            .x = 0, .y = 2, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<21>()},
        UiWidgetBlueprint{
            .x = 0, .y = 3, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<20>()},
        UiWidgetBlueprint{
            .x = 0, .y = 4, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<19>()},
        UiWidgetBlueprint{
            .x = 0, .y = 5, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<18>()},
        UiWidgetBlueprint{
            .x = 0, .y = 6, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<17>()},
        UiWidgetBlueprint{
            .x = 0, .y = 7, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<16>()},
        UiWidgetBlueprint{
            .x = 0, .y = 8, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<15>()},
        UiWidgetBlueprint{
            .x = 0, .y = 9, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<14>()},
        UiWidgetBlueprint{
            .x = 0, .y = 10, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<13>()},
        UiWidgetBlueprint{
            .x = 0, .y = 11, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<12>()},
        UiWidgetBlueprint{
            .x = 0, .y = 12, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<11>()},
        UiWidgetBlueprint{
            .x = 0, .y = 13, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<10>()},
        UiWidgetBlueprint{
            .x = 0, .y = 14, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<9>()},
        UiWidgetBlueprint{
            .x = 0, .y = 15, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<8>()},
        UiWidgetBlueprint{
            .x = 0, .y = 16, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<7>()},
        UiWidgetBlueprint{
            .x = 0, .y = 17, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<6>()},
        UiWidgetBlueprint{
            .x = 0, .y = 18, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<5>()},
        UiWidgetBlueprint{
            .x = 0, .y = 19, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<4>()},
        UiWidgetBlueprint{
            .x = 0, .y = 20, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<3>()},
        UiWidgetBlueprint{
            .x = 0, .y = 21, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<2>()},
        UiWidgetBlueprint{
            .x = 0, .y = 22, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<1>()},
        UiWidgetBlueprint{
            .x = 0, .y = 23, .w = 1, .h = 1, .border_width = 0, .variant = command_output_variant<0>()},
        UiWidgetBlueprint{
            .initially_focused = true,
            .x = 0,
            .y = 24,
            .w = 1,
            .h = 1,
            .background_color = SDL_Color{27, 27, 27, 255},
            .variant = widget::TextInputBlueprint{
                .initial_text = "", // May be overriden with console_command on startup
                .onAction = [](const widget::TextInput &self) -> std::pair<UiAction, std::optional<std::string>> {
	                const auto &command = self.text_input();
	                return std::make_pair(handle_console_command(command), std::string{});
                }}}}};
