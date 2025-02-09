#include <m2/sheet_editor/Ui.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/Image.h>

using namespace m2;
using namespace m2;

const widget::TextBlueprint right_hud_set_button = {
		.text = "Set",
		.onAction = [](MAYBE const widget::Text& self) -> UiAction {
			std::visit(m2::overloaded {
					[](sheet_editor::State::BackgroundColliderMode& mode) { mode.set(); },
					[](sheet_editor::State::ForegroundColliderMode& mode) { mode.set(); },
					[](const auto&) {},
			}, std::get<sheet_editor::State>(M2_LEVEL.stateVariant).mode);
			return MakeContinueAction();
		}
};
const widget::TextBlueprint right_hud_set_rect_button = {
		.text = "Set Rect",
		.onAction = [](MAYBE const widget::Text& self) -> UiAction {
			std::visit(m2::overloaded {
					[](sheet_editor::State::ForegroundCompanionMode& mode) { mode.add_rect(); },
					[](sheet_editor::State::RectMode& mode) { mode.set_rect(); },
					[](const auto&) {},
			}, std::get<sheet_editor::State>(M2_LEVEL.stateVariant).mode);
			return MakeContinueAction();
		}
};
const widget::TextBlueprint right_hud_set_center_button = {
		.text = "Set Center",
		.onAction = [](MAYBE const widget::Text& self) -> UiAction {
			std::visit(m2::overloaded {
					[](sheet_editor::State::ForegroundCompanionMode& mode) { mode.set_center(); },
					[](sheet_editor::State::RectMode& mode) { mode.set_center(); },
					[](const auto&) {},
			}, std::get<sheet_editor::State>(M2_LEVEL.stateVariant).mode);
			return MakeContinueAction();
		}
};
const widget::TextBlueprint right_hud_reset_button = {
		.text = "Reset",
		.onAction = [](MAYBE const widget::Text& self) -> UiAction {
			std::visit(m2::overloaded {
				[](sheet_editor::State::ForegroundCompanionMode& mode) { mode.reset(); },
				[](sheet_editor::State::RectMode& mode) { mode.reset(); },
				[](sheet_editor::State::BackgroundColliderMode& mode) { mode.reset(); },
				[](sheet_editor::State::ForegroundColliderMode& mode) { mode.reset(); },
				[](const auto&) {},
			}, std::get<sheet_editor::State>(M2_LEVEL.stateVariant).mode);
			return MakeContinueAction();
		}
};

UiPanelBlueprint sheet_editor_foreground_companion_mode_right_hud = {
	.name = "ForegroundCompanionModeRightHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				UiWidgetBlueprint{
					.x = 4, .y = 2, .w = 11, .h = 3,
					.border_width = 0,
					.variant = widget::TextBlueprint{
						.text = "FComp"
					}
				},
				UiWidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.variant = right_hud_set_rect_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.variant = right_hud_set_center_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 4,
						.variant = right_hud_reset_button
				}
		}
};

UiPanelBlueprint sheet_editor_rect_mode_right_hud = {
	.name = "RectModeRightHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				UiWidgetBlueprint{
					.x = 4, .y = 2, .w = 11, .h = 3,
					.border_width = 0,
					.variant = widget::TextBlueprint{
						.text = "Rect"
					}
				},
				UiWidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.variant = right_hud_set_rect_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.variant = right_hud_set_center_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 16, .w = 11, .h = 4,
						.variant = right_hud_reset_button
				}
		}
};

UiPanelBlueprint sheet_editor_background_collider_mode_right_hud = {
	.name = "BackgroundColliderModeRightHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				UiWidgetBlueprint{
					.x = 4, .y = 2, .w = 11, .h = 3,
					.border_width = 0,
					.variant = widget::TextBlueprint{
						.text = "BColl"
					}
				},
				UiWidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.variant = right_hud_set_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.variant = right_hud_reset_button
				}
		}
};

UiPanelBlueprint sheet_editor_foreground_collider_mode_right_hud = {
	.name = "ForegroundColliderModeRightHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				UiWidgetBlueprint{
					.x = 4, .y = 2, .w = 11, .h = 3,
					.border_width = 0,
					.variant = widget::TextBlueprint{
						.text = "FColl"
					}
				},
				UiWidgetBlueprint{
						.x = 4, .y = 6, .w = 11, .h = 4,
						.variant = right_hud_set_button
				},
				UiWidgetBlueprint{
						.x = 4, .y = 11, .w = 11, .h = 4,
						.variant = right_hud_reset_button
				}
		}
};

const UiPanelBlueprint m2::sheet_editor_right_hud = {
	.name = "RightHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.widgets = {}
};

const UiPanelBlueprint m2::sheet_editor_left_hud = {
	.name = "LeftHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				UiWidgetBlueprint{
					.x = 4, .y = 2, .w = 11, .h = 3,
					.variant = widget::TextBlueprint{
						.text = "FComp",
						.onAction = [](MAYBE const widget::Text& self) -> UiAction {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).activate_foreground_companion_mode();

							M2_LEVEL.ReplaceRightHud(&sheet_editor_foreground_companion_mode_right_hud, M2_GAME.Dimensions().RightHud());
							return MakeContinueAction();
						}
					}
				},
				UiWidgetBlueprint{
					.x = 4, .y = 6, .w = 11, .h = 3,
					.variant = widget::TextBlueprint{
						.text = "Rect",
						.onAction = [](MAYBE const widget::Text& self) -> UiAction {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).activate_rect_mode();

							M2_LEVEL.ReplaceRightHud(&sheet_editor_rect_mode_right_hud, M2_GAME.Dimensions().RightHud());
							return MakeContinueAction();
						}
					}
				},
				UiWidgetBlueprint{
					.x = 4, .y = 10, .w = 11, .h = 3,
					.variant = widget::TextBlueprint{
						.text = "BColl",
						.onAction = [](MAYBE const widget::Text& self) -> UiAction {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).activate_background_collider_mode();

							M2_LEVEL.ReplaceRightHud(&sheet_editor_background_collider_mode_right_hud, M2_GAME.Dimensions().RightHud());
							return MakeContinueAction();
						}
					}
				},
				UiWidgetBlueprint{
					.x = 4, .y = 14, .w = 11, .h = 3,
					.variant = widget::TextBlueprint{
						.text = "FColl",
						.onAction = [](MAYBE const widget::Text& self) -> UiAction {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).activate_foreground_collider_mode();

							M2_LEVEL.ReplaceRightHud(&sheet_editor_foreground_collider_mode_right_hud, M2_GAME.Dimensions().RightHud());
							return MakeContinueAction();
						}
					}
				},
				UiWidgetBlueprint{
					.x = 4, .y = 18, .w = 11, .h = 3,
					.variant = widget::TextBlueprint{
						.text = "Cancel",
						.onAction = [](MAYBE const widget::Text& self) -> UiAction {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).deactivate_mode();

							M2_LEVEL.ReplaceRightHud(&sheet_editor_right_hud, M2_GAME.Dimensions().RightHud());
							return MakeContinueAction();
						}
					}
				},
				UiWidgetBlueprint{
					.x = 0, .y = 70, .w = 19, .h = 2,
					.border_width = 0,
					.variant = widget::TextBlueprint{
						.text = "0.0:0.0",
						.onUpdate = [](MAYBE widget::Text& self) {
							auto mouse_position = M2_GAME.MousePositionWorldM().hround();
							self.set_text(m2::ToString(mouse_position.x, 1) + ':' + m2::ToString(mouse_position.y, 1));
							return MakeContinueAction();
						}
					}
				}
		}
};

const UiPanelBlueprint m2::sheet_editor_main_menu = {
	.name = "MainMenu",
		.w = 160, .h = 90,
		.border_width = 0,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				UiWidgetBlueprint{
					.x = 60, .y = 10, .w = 40, .h = 40,
					.variant = widget::ImageBlueprint{
						.onUpdate = [](const widget::Image& self) -> std::pair<UiAction,std::optional<m2g::pb::SpriteType>> {
							auto* text_selection_widget = self.Parent().find_first_widget_of_blueprint_type<widget::TextSelectionBlueprint>();
							m2g::pb::SpriteType selected_sprite_type = static_cast<m2g::pb::SpriteType>(
								std::get<int>(dynamic_cast<widget::TextSelection*>(text_selection_widget)->selections()[0]));
							return std::make_pair(MakeContinueAction(), selected_sprite_type);
						}
					}
				},
				UiWidgetBlueprint{
					.x = 35 , .y = 55, .w = 90, .h = 10,
					.variant = widget::TextSelectionBlueprint{
						.onCreate = [](MAYBE widget::TextSelection& self) {
							const auto& pb_sheets = std::get<sheet_editor::State>(M2_LEVEL.stateVariant).sprite_sheets();
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
							widget::TextSelectionBlueprint::Options options;
							std::transform(sprite_types.begin(), sprite_types.end(), std::back_inserter(options), [](const auto& sprite_type) {
								return widget::TextSelectionBlueprint::Option{m2g::pb::SpriteType_Name(sprite_type), static_cast<int>(sprite_type)};
							});
							self.set_options(std::move(options));
						},
						.onAction = [](widget::TextSelection& self) -> UiAction {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).set_sprite_type(
								static_cast<m2g::pb::SpriteType>(std::get<int>(self.selections()[0])));
							return MakeContinueAction();
						}
					}
				},
				UiWidgetBlueprint{
						.x = 35, .y = 70, .w = 40, .h = 10,
						.variant = widget::TextBlueprint{
								.text = "QUIT",
								.kb_shortcut = SDL_SCANCODE_Q,
								.onAction = [](MAYBE const widget::Text& self) -> UiAction { return MakeQuitAction(); },
						}
				},
				UiWidgetBlueprint{
						.x = 85, .y = 70, .w = 40, .h = 10,
						.variant = widget::TextBlueprint{
								.text = "SELECT",
								.kb_shortcut = SDL_SCANCODE_RETURN,
								.onAction = [](MAYBE const widget::Text& self) -> UiAction {
									if (not std::holds_alternative<std::monostate>(std::get<sheet_editor::State>(M2_LEVEL.stateVariant).mode)) {
										std::get<sheet_editor::State>(M2_LEVEL.stateVariant).deactivate_mode();
										M2_LEVEL.ReplaceRightHud(&sheet_editor_right_hud, M2_GAME.Dimensions().RightHud());
									}
									std::get<sheet_editor::State>(M2_LEVEL.stateVariant).select();
									return MakeReturnAction(); // TODO Return value
								}
						}
				}
		}
};
