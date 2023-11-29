#include <m2/sheet_editor/Ui.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Image.h>

using namespace m2;
using namespace m2::ui;

const widget::TextBlueprint right_hud_set_button = {
		.initial_text = "Set",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::visit(m2::overloaded {
					[](sedit::State::BackgroundColliderMode& mode) { mode.set(); },
					[](sedit::State::ForegroundColliderMode& mode) { mode.set(); },
					[](const auto&) {},
			}, std::get<sedit::State>(LEVEL.type_state).mode);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint right_hud_set_rect_button = {
		.initial_text = "Set Rect",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::visit(m2::overloaded {
					[](sedit::State::ForegroundCompanionMode& mode) { mode.add_rect(); },
					[](sedit::State::RectMode& mode) { mode.set_rect(); },
					[](const auto&) {},
			}, std::get<sedit::State>(LEVEL.type_state).mode);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint right_hud_set_center_button = {
		.initial_text = "Set Center",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::visit(m2::overloaded {
					[](sedit::State::ForegroundCompanionMode& mode) { mode.set_center(); },
					[](sedit::State::RectMode& mode) { mode.set_center(); },
					[](const auto&) {},
			}, std::get<sedit::State>(LEVEL.type_state).mode);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint right_hud_reset_button = {
		.initial_text = "Reset",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::visit(m2::overloaded {
				[](sedit::State::ForegroundCompanionMode& mode) { mode.reset(); },
				[](sedit::State::RectMode& mode) { mode.reset(); },
				[](sedit::State::BackgroundColliderMode& mode) { mode.reset(); },
				[](sedit::State::ForegroundColliderMode& mode) { mode.reset(); },
				[](const auto&) {},
			}, std::get<sedit::State>(LEVEL.type_state).mode);
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
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<sedit::State>(LEVEL.type_state).activate_foreground_companion_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_foreground_companion_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_rect_button = {
		.initial_text = "Rect",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<sedit::State>(LEVEL.type_state).activate_rect_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_rect_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_background_collider_button = {
		.initial_text = "BColl",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<sedit::State>(LEVEL.type_state).activate_background_collider_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_background_collider_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_foreground_collider_button = {
		.initial_text = "FColl",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<sedit::State>(LEVEL.type_state).activate_foreground_collider_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_foreground_collider_mode_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_cancel_button = {
		.initial_text = "Cancel",
		.on_action = [](MAYBE const widget::Text& self) -> Action {
			std::get<sedit::State>(LEVEL.type_state).deactivate_mode();

			LEVEL.right_hud_ui_state.emplace(&sheet_editor_right_hud);
			LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
			return Action::CONTINUE;
		}
};
const widget::TextBlueprint left_hud_coordinates = {
		.initial_text = "0.0:0.0",
		.on_update = [](MAYBE const widget::Text& self) {
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

widget::ImageBlueprint sprite_display = {
		.on_update = [](const widget::Image& self) -> std::pair<Action,std::optional<m2g::pb::SpriteType>> {
			auto* text_selection_widget = self.parent().find_first_widget_of_blueprint_type<widget::TextSelectionBlueprint>();
			auto selected_sprite_name = dynamic_cast<widget::TextSelection*>(text_selection_widget)->selection();
			m2g::pb::SpriteType selected_sprite_type;
			if (m2g::pb::SpriteType_Parse(selected_sprite_name, &selected_sprite_type)) {
				return std::make_pair(Action::CONTINUE, selected_sprite_type);
			} else {
				throw M2FATAL("Implementation error: Unknown sprite type ended up in sprite selection list");
			}
		}
};
const widget::TextSelectionBlueprint sprite_selection = {
		.on_create = [](MAYBE const widget::TextSelection& self) -> std::optional<widget::TextSelectionBlueprint::Options> {
			const auto& pb_sheets = std::get<sedit::State>(LEVEL.type_state).sprite_sheets();
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
		.on_action = [](const widget::TextSelection& self) -> ui::Action {
			m2g::pb::SpriteType selected_sprite_type;
			if (m2g::pb::SpriteType_Parse(self.selection(), &selected_sprite_type)) {
				std::get<sedit::State>(LEVEL.type_state).set_sprite_type(selected_sprite_type);
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
					.x = 60, .y = 10, .w = 40, .h = 40,
					.border_width_px = 1,
					.variant = sprite_display
				},
				WidgetBlueprint{
						.x = 35 , .y = 55, .w = 90, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = sprite_selection
				},
				WidgetBlueprint{
						.x = 35, .y = 70, .w = 40, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = widget::TextBlueprint{
								.initial_text = "QUIT",
								.kb_shortcut = SDL_SCANCODE_Q,
								.on_action = [](MAYBE const widget::Text& self) -> Action { return Action::QUIT; },
						}
				},
				WidgetBlueprint{
						.x = 85, .y = 70, .w = 40, .h = 10,
						.border_width_px = 1,
						.padding_width_px = 2,
						.variant = widget::TextBlueprint{
								.initial_text = "SELECT",
								.kb_shortcut = SDL_SCANCODE_RETURN,
								.on_action = [](MAYBE const widget::Text& self) -> Action {
									if (not std::holds_alternative<std::monostate>(std::get<sedit::State>(LEVEL.type_state).mode)) {
										std::get<sedit::State>(LEVEL.type_state).deactivate_mode();
										LEVEL.right_hud_ui_state.emplace(&sheet_editor_right_hud);
										LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
									}
									std::get<sedit::State>(LEVEL.type_state).select();
									return Action::RETURN;
								}
						}
				}
		}
};
