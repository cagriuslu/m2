#include <m2/sheet_editor/Ui.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;

const widget::TextBlueprint right_hud_set_button = {
		.initial_text = "Set",
		.action_callback = []() -> Action {
			std::visit(m2::overloaded {
					[](sedit::State::BackgroundColliderMode& mode) { mode.set(); },
					[](sedit::State::ForegroundColliderMode& mode) { mode.set(); },
					[](const auto&) {},
			}, LEVEL.sheet_editor_state->mode);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint right_hud_set_rect_button = {
		.initial_text = "Set Rect",
		.action_callback = []() -> Action {
			std::visit(m2::overloaded {
					[](sedit::State::ForegroundCompanionMode& mode) { mode.add_rect(); },
					[](sedit::State::RectMode& mode) { mode.set_rect(); },
					[](const auto&) {},
			}, LEVEL.sheet_editor_state->mode);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint right_hud_set_center_button = {
		.initial_text = "Set Center",
		.action_callback = []() -> Action {
			std::visit(m2::overloaded {
					[](sedit::State::ForegroundCompanionMode& mode) { mode.set_center(); },
					[](sedit::State::RectMode& mode) { mode.set_center(); },
					[](const auto&) {},
			}, LEVEL.sheet_editor_state->mode);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint right_hud_reset_button = {
		.initial_text = "Reset",
		.action_callback = []() -> Action {
			std::visit(m2::overloaded {
				[](sedit::State::ForegroundCompanionMode& mode) { mode.reset(); },
				[](sedit::State::RectMode& mode) { mode.reset(); },
				[](sedit::State::BackgroundColliderMode& mode) { mode.reset(); },
				[](sedit::State::ForegroundColliderMode& mode) { mode.reset(); },
				[](const auto&) {},
			}, LEVEL.sheet_editor_state->mode);
			return Action::CONTINUE;
		}
};

const widget::TextBlueprint foreground_companion_mode_title = {
		.initial_text = "FComp"
};
Blueprint sheet_editor_foreground_companion_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = foreground_companion_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_set_rect_button
				},
				WidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_set_center_button
				},
				WidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_reset_button
				}
		}
};

const widget::TextBlueprint rect_mode_title = {
		.initial_text = "Rect"
};
Blueprint sheet_editor_rect_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = rect_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_set_rect_button
				},
				WidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_set_center_button
				},
				WidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_reset_button
				}
		}
};

const widget::TextBlueprint background_collider_mode_title = {
		.initial_text = "BColl"
};
Blueprint sheet_editor_background_collider_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = background_collider_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_set_button
				},
				WidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_reset_button
				}
		}
};

const widget::TextBlueprint foreground_collider_mode_title = {
		.initial_text = "FColl"
};
Blueprint sheet_editor_foreground_collider_mode_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				WidgetBlueprint{
						.x = 4, .y = 2, .w = 11, .h = 3,
						.border_width_px = 0,
						.variant = foreground_collider_mode_title
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_set_button
				},
				WidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.border_width_px = 1,
						.variant = right_hud_reset_button
				}
		}
};

const ui::Blueprint m2::ui::sheet_editor_right_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {}
};

const widget::TextBlueprint left_hud_foreground_companion_button = {
		.initial_text = "FComp",
		.action_callback = []() -> Action {
			LEVEL.sheet_editor_state->activate_foreground_companion_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_foreground_companion_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_rect_button = {
		.initial_text = "Rect",
		.action_callback = []() -> Action {
			LEVEL.sheet_editor_state->activate_rect_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_rect_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_background_collider_button = {
		.initial_text = "BColl",
		.action_callback = []() -> Action {
			LEVEL.sheet_editor_state->activate_background_collider_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_background_collider_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_foreground_collider_button = {
		.initial_text = "FColl",
		.action_callback = []() -> Action {
			LEVEL.sheet_editor_state->activate_foreground_collider_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_foreground_collider_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_cancel_button = {
		.initial_text = "Cancel",
		.action_callback = []() -> Action {
			LEVEL.sheet_editor_state->deactivate_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_coordinates = {
		.initial_text = "0.0:0.0",
		.update_callback = []() {
			auto mouse_position = GAME.mouse_position_world_m().hround();
			return std::make_pair(Action::CONTINUE, m2::to_string(mouse_position.x, 1) + ':' + m2::to_string(mouse_position.y, 1));
		}
};
const Blueprint m2::ui::sheet_editor_left_hud = {
		.w = 19, .h = 72,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				WidgetBlueprint{
					.x = 4, .y = 2, .w = 11, .h = 3,
					.border_width_px = 1,
					.padding_width_px = 2,
					.variant = left_hud_foreground_companion_button
				},
				WidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_rect_button
				},
				WidgetBlueprint{
						.x = 4, .y = 10, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_background_collider_button
				},
				WidgetBlueprint{
						.x = 4, .y = 14, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_foreground_collider_button
				},
				WidgetBlueprint{
						.x = 4, .y = 18, .w = 11, .h = 3,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = left_hud_cancel_button
				},
				WidgetBlueprint{
						.x = 0, .y = 70, .w = 19, .h = 2,
						.border_width_px = 0,
						.variant = left_hud_coordinates
				}
		}
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
