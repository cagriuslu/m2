#include <m2/sheet_editor/Ui.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/Image.h>

using namespace m2;

namespace {
	const UiPanelBlueprint rectModeRightHud = {
		.name = "RectModeRightHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.onCreate = [](UiPanel&) {
			M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());
			M2_LEVEL.EnableSecondarySelection(M2_GAME.Dimensions().Game());
		},
		.onDestroy = [] {
			M2_LEVEL.DisablePrimarySelection();
			M2_LEVEL.DisableSecondarySelection();
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1, .y = 2, .w = 17, .h = 3,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = "Rect"
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 6, .w = 17, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Set Rect",
					.onAction = [](MAYBE const widget::Text& self) -> UiAction {
						if (auto* selection = M2_LEVEL.PrimarySelection(); selection->IsComplete()) {
							const auto intSelection = selection->IntegerSelectionRectM();
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).SetSpriteRect(*intSelection);
							selection->Reset();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 11, .w = 17, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Set Center",
					.onAction = [](MAYBE const widget::Text& self) -> UiAction {
						if (auto* selection = M2_LEVEL.SecondarySelection(); selection->IsComplete()) {
							const auto originSelection = selection->HalfCellSelectionsM();
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).SetSpriteOrigin(originSelection->first);
							selection->Reset();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 16, .w = 17, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Reset",
					.onAction = [](MAYBE const widget::Text& self) -> UiAction {
						std::get<sheet_editor::State>(M2_LEVEL.stateVariant).ResetSpriteRectAndOrigin();
						M2_LEVEL.PrimarySelection()->Reset();
						M2_LEVEL.SecondarySelection()->Reset();
						return MakeContinueAction();
					}
				}
			}
		}
	};

	const UiPanelBlueprint foregroundCompanionModeRightHud = {
		.name = "ForegroundCompanionModeRightHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.onCreate = [](UiPanel&) {
			M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());
			M2_LEVEL.EnableSecondarySelection(M2_GAME.Dimensions().Game());
		},
		.onDestroy = [] {
			M2_LEVEL.DisablePrimarySelection();
			M2_LEVEL.DisableSecondarySelection();
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1, .y = 2, .w = 17, .h = 3,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = "Foreground Companion"
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 6, .w = 17, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Set Rect",
					.onAction = [](MAYBE const widget::Text& self) -> UiAction {
						if (auto* selection = M2_LEVEL.PrimarySelection(); selection->IsComplete()) {
							const auto intSelection = selection->IntegerSelectionRectM();
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).AddForegroundCompanionRect(*intSelection);
							selection->Reset();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 11, .w = 17, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Set Center",
					.onAction = [](MAYBE const widget::Text& self) -> UiAction {
						if (auto* selection = M2_LEVEL.SecondarySelection(); selection->IsComplete()) {
							const auto originSelection = selection->HalfCellSelectionsM();
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).SetForegroundCompanionOrigin(originSelection->first);
							selection->Reset();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 16, .w = 17, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Reset",
					.onAction = [](MAYBE const widget::Text& self) -> UiAction {
						std::get<sheet_editor::State>(M2_LEVEL.stateVariant).ResetForegroundCompanion();
						M2_LEVEL.PrimarySelection()->Reset();
						M2_LEVEL.SecondarySelection()->Reset();
						return MakeContinueAction();
					}
				}
			}
		}
	};

	UiPanelBlueprint fixtureModeRightHud = {
		.name = "FixtureModeRightHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.onCreate = [](UiPanel&) {
			M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());
		},
		.onDestroy = [] {
			M2_LEVEL.DisablePrimarySelection();
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1, .y = 2, .w = 17, .h = 3,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = "Fixture"
				}
			},
			UiWidgetBlueprint{
				.name = "LayerSelection",
				.x = 1, .y = 6, .w = 17, .h = 3,
				.variant = widget::TextSelectionBlueprint{
					.options = {
						{.text = "Background", .return_value = FIXTURE_LAYER_SELECTION_BACKGROUND_OPTION},
						{.text = "Foreground", .return_value = FIXTURE_LAYER_SELECTION_FOREGROUND_OPTION}}
				}
			},
			UiWidgetBlueprint{
				.name = "ShapeSelection",
				.x = 1, .y = 10, .w = 17, .h = 3,
				.variant = widget::TextSelectionBlueprint{
					.options = {
						{.text = "Rectangle", .return_value = FIXTURE_SHAPE_SELECTION_RECTANGLE},
						{.text = "Circle", .return_value = FIXTURE_SHAPE_SELECTION_CIRCLE},
						{.text = "Chain Point", .return_value = FIXTURE_SHAPE_SELECTION_CHAIN_POINT}}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 14, .w = 17, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Add",
					.onAction = [](const widget::Text& self) -> UiAction {
						if (auto* selection = M2_LEVEL.PrimarySelection(); selection->IsComplete()) {
							const auto foreground = std::get<int>(self.Parent().find_first_widget_by_name<widget::TextSelection>("LayerSelection")->selections()[0]) == FIXTURE_LAYER_SELECTION_FOREGROUND_OPTION;
							if (const auto shape = std::get<int>(self.Parent().find_first_widget_by_name<widget::TextSelection>("ShapeSelection")->selections()[0]);
									shape == FIXTURE_SHAPE_SELECTION_RECTANGLE) {
								const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionRectM();
								std::get<sheet_editor::State>(M2_LEVEL.stateVariant).AddRectangleFixture(foreground, *halfCellSelection);
							} else if (shape == FIXTURE_SHAPE_SELECTION_CIRCLE) {
								const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionsM();
								const auto center = halfCellSelection->first;
								const auto radius = center.distance(halfCellSelection->second);
								std::get<sheet_editor::State>(M2_LEVEL.stateVariant).AddCircleFixture(foreground, center, radius);
							} else if (shape == FIXTURE_SHAPE_SELECTION_CHAIN_POINT) {
								const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionsM();
								std::get<sheet_editor::State>(M2_LEVEL.stateVariant).AddChainFixturePoint(foreground, halfCellSelection->first);
							}
							selection->Reset();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 19, .w = 17, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Reset",
					.onAction = [](MAYBE const widget::Text& self) -> UiAction {
						const auto foreground = std::get<int>(self.Parent().find_first_widget_by_name<widget::TextSelection>("LayerSelection")->selections()[0]) == FIXTURE_LAYER_SELECTION_FOREGROUND_OPTION;
						if (const auto shape = std::get<int>(self.Parent().find_first_widget_by_name<widget::TextSelection>("ShapeSelection")->selections()[0]);
								shape == FIXTURE_SHAPE_SELECTION_RECTANGLE) {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).ResetRectangleFixtures(foreground);
						} else if (shape == FIXTURE_SHAPE_SELECTION_CIRCLE) {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).ResetCircleFixtures(foreground);
						} else if (shape == FIXTURE_SHAPE_SELECTION_CHAIN_POINT) {
							std::get<sheet_editor::State>(M2_LEVEL.stateVariant).ResetChainFixturePoints(foreground);
						}
						M2_LEVEL.PrimarySelection()->Reset();
						return MakeContinueAction();
					}
				}
			}
		}
	};
}

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
			.x = 1, .y = 2, .w = 17, .h = 8,
			.variant = widget::TextBlueprint{
				.text = "Foreground Companion",
				.wrapped_font_size_in_units = 2.4f,
				.onAction = [](MAYBE const widget::Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&foregroundCompanionModeRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 1, .y = 11, .w = 17, .h = 8,
			.variant = widget::TextBlueprint{
				.text = "Rect",
				.wrapped_font_size_in_units = 2.4f,
				.onAction = [](MAYBE const widget::Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&rectModeRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 1, .y = 20, .w = 17, .h = 8,
			.variant = widget::TextBlueprint{
				.text = "Fixture",
				.wrapped_font_size_in_units = 2.4f,
				.onAction = [](MAYBE const widget::Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&fixtureModeRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 1, .y = 29, .w = 17, .h = 4,
			.variant = widget::TextBlueprint{
				.text = "Cancel",
				.wrapped_font_size_in_units = 2.4f,
				.onAction = [](MAYBE const widget::Text& self) -> UiAction {
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
					const auto mouse_position = M2_GAME.MousePositionWorldM().hround();
					self.set_text(ToString(mouse_position.x, 1) + ':' + ToString(mouse_position.y, 1));
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
			.name = "SpriteDisplay",
			.x = 60, .y = 10, .w = 40, .h = 40,
			.variant = widget::ImageBlueprint{}
		},
		UiWidgetBlueprint{
			.name = "SpriteTypeSelection",
			.x = 35 , .y = 55, .w = 90, .h = 10,
			.variant = widget::TextSelectionBlueprint{
				.onCreate = [](widget::TextSelection& self) {
					const auto& pb_sheets = std::get<sheet_editor::State>(M2_LEVEL.stateVariant).SpriteSheets();
					// Gather the list of sprites
					std::vector<m2g::pb::SpriteType> sprite_types;
					std::ranges::for_each(pb_sheets.sheets(), [&sprite_types](const auto& sheet) {
						std::for_each(sheet.sprites().cbegin(), sheet.sprites().cend(), [&sprite_types](const auto& sprite) {
							sprite_types.emplace_back(sprite.type());
						});
					});
					// Sort the list
					std::ranges::sort(sprite_types);
					// Transform to sprite type names
					widget::TextSelectionBlueprint::Options options;
					std::ranges::transform(sprite_types, std::back_inserter(options), [](const auto& sprite_type) {
						return widget::TextSelectionBlueprint::Option{m2g::pb::SpriteType_Name(sprite_type), static_cast<int>(sprite_type)};
					});
					self.set_options(std::move(options));
				},
				.onAction = [](const widget::TextSelection& self) {
					if (const auto selections = self.selections(); not selections.empty()) {
						const auto selectedSprite = static_cast<m2g::pb::SpriteType>(std::get<int>(selections[0]));
						self.Parent().find_first_widget_by_name<widget::Image>("SpriteDisplay")->SetSpriteType(selectedSprite);
					}
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 35, .y = 70, .w = 40, .h = 10,
			.variant = widget::TextBlueprint{
				.text = "QUIT",
				.onAction = [](MAYBE const widget::Text& self) -> UiAction { return MakeQuitAction(); },
			}
		},
		UiWidgetBlueprint{
			.x = 85, .y = 70, .w = 40, .h = 10,
			.variant = widget::TextBlueprint{
				.text = "SELECT",
				.onAction = [](MAYBE const widget::Text& self) -> UiAction {
					if (const auto selections = self.Parent().find_first_widget_by_name<widget::TextSelection>("SpriteTypeSelection")->selections();
							not selections.empty()) {
						M2_LEVEL.ReplaceRightHud(&sheet_editor_right_hud, M2_GAME.Dimensions().RightHud());
						const auto selectedSprite = static_cast<m2g::pb::SpriteType>(std::get<int>(selections[0]));
						std::get<sheet_editor::State>(M2_LEVEL.stateVariant).Select(selectedSprite);
						return MakeReturnAction();
					}
					return MakeContinueAction();
				}
			}
		}
	}
};
