#include <m2/sheet_editor/Ui.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;

const Blueprint m2::ui::sheet_editor_left_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {}
};

const ui::Blueprint m2::ui::sheet_editor_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {}
};

const widget::TextSelectionBlueprint sprite_selection = {
		.on_create = []() -> std::optional<widget::TextSelectionBlueprint::Options> {
			const auto& pb_sheets = LEVEL.sheet_editor_state->sprite_sheets();
			// Gather the list of sprites
			std::vector<m2g::pb::SpriteType> sprite_types;
			std::for_each(pb_sheets.sheets().cbegin(), pb_sheets.sheets().cend(), [&sprite_types](const auto& sheet) {
				std::for_each(sheet.sprites().cbegin(), sheet.sprites().cend(), [&sprite_types](const auto& sprite) {
					sprite_types.emplace_back(sprite.type());
				});
			});
			// Sort the list
			std::sort(sprite_types.begin(), sprite_types.end());
			// Transform to sprite type names
			widget::TextSelectionBlueprint::Options sprite_type_names;
			std::transform(sprite_types.cbegin(), sprite_types.cend(), std::back_inserter(sprite_type_names), [](const auto& sprite_type) {
				return m2g::pb::SpriteType_Name(sprite_type);
			});
			return sprite_type_names;
		},
		.action_callback = [](MAYBE unsigned selection_idx, const std::string& selection) -> ui::Action {
			m2g::pb::SpriteType selected_sprite_type;
			if (m2g::pb::SpriteType_Parse(selection, &selected_sprite_type)) {
				LEVEL.sheet_editor_state->select_sprite_type(selected_sprite_type);
				return Action::CONTINUE;
			} else {
				throw M2FATAL("Implementation error: Unknown sprite type ended up in sprite selection list");
			}
		}
};
const Blueprint m2::ui::sheet_editor_main_menu = {
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 35 , .y = 20, .w = 90, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = sprite_selection
				},
				WidgetBlueprint{
						.x = 35, .y = 60, .w = 40, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = widget::TextBlueprint{
								.initial_text = "QUIT",
								.action_callback = []() -> Action { return Action::QUIT; },
								.kb_shortcut = SDL_SCANCODE_Q
						}
				},
				WidgetBlueprint{
						.x = 85, .y = 60, .w = 40, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = widget::TextBlueprint{
								.initial_text = "SELECT",
								.action_callback = []() -> Action {
									LEVEL.sheet_editor_state->prepare_sprite_selection();
									return Action::RETURN;
								},
								.kb_shortcut = SDL_SCANCODE_RETURN
						}
				}
		}
};
