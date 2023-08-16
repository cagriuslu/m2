#include <m2/sheet_editor/Ui.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;

const Blueprint m2::ui::sheet_editor_left_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {}
};

const ui::Blueprint m2::ui::sheet_editor_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.widgets = {}
};

const widget::TextBlueprint add_sheet_button = {
		.initial_text = "Add Sheet",
		.action_callback = []() -> Action {
			auto action = LEVEL.sheet_editor_state->execute_add_sheet_dialog();
			return action == Action::QUIT ? action : Action::CONTINUE; // Propagate only the QUIT event
		}
};
const widget::TextSelectionBlueprint sheet_selection = {
		.update_callback = [](const std::vector<std::string>& current_list, MAYBE unsigned current_selection) -> std::pair<Action,std::optional<std::vector<std::string>>> {
			const auto& sheets_pb = LEVEL.sheet_editor_state->sheets_pb;
			// Gather the list of sheets
			std::vector<std::string> sheet_filenames;
			for (const auto& sheet : sheets_pb.sheets()) {
				auto path = std::filesystem::path{sheet.resource()};
				sheet_filenames.emplace_back(path.filename());
			}
			// Compare the list, return only if there's difference
			if (is_identical(current_list.begin(), current_list.end(), sheet_filenames.cbegin(), sheet_filenames.cend())) {
				return std::make_pair(Action::CONTINUE, std::nullopt);
			} else {
				return std::make_pair(Action::CONTINUE, sheet_filenames);
			}
		},
		.action_callback = [](unsigned selection_idx, MAYBE const std::string& selection) -> ui::Action {
			LEVEL.sheet_editor_state->selected_sheet = selection_idx;
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint edit_sheet_button = {
		.initial_text = "Edit Sheet",
		.action_callback = []() -> Action {
			auto action = LEVEL.sheet_editor_state->execute_add_sheet_dialog(true);
			return action == Action::QUIT ? action : Action::CONTINUE; // Propagate only the QUIT event
		}
};
const Blueprint m2::ui::sheet_editor_main_menu = {
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 4, .w = 48, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = add_sheet_button
				},
				WidgetBlueprint{
						.x = 4, .y = 18, .w = 48, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = sheet_selection
				},
				WidgetBlueprint{
						.x = 4, .y = 32, .w = 48, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = edit_sheet_button
				},
				WidgetBlueprint{
						.x = 108, .y = 76, .w = 48, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = widget::TextBlueprint{
								.initial_text = "QUIT",
								.action_callback = []() -> Action { return Action::QUIT; },
								.kb_shortcut = SDL_SCANCODE_Q
						}
				},
		}
};
