#include <m2/level_editor/Ui.h>
#include <m2/sheet_editor/Ui.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/CheckboxWithTextBlueprint.h>
#include <m2/protobuf/Detail.h>
#include <m2/game/object/Ghost.h>
#include <m2/ui/Layout.h>
#include <m2/Game.h>
#include <m2/game/Selection.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/sheet_editor/PersistentSpriteSheets.h>

using namespace m2;
using namespace m2::widget;

namespace {
	UiPanelBlueprint CreateTextTooltipBlueprint(const std::string& s) {
		return UiPanelBlueprint{
			.name = "Tooltip",
			.background_color = {0, 0, 0, 255},
			.widgets = {UiWidgetBlueprint{
				.variant = TextBlueprint{
					.text = s
				}
			}}
		};
	}
	const RectF gTextTooltipRatio{0.0f, 0.0f, 0.2f, 0.05f};

	std::vector<m2g::pb::ObjectType> ObjectTypesWithMainSprite() {
		std::vector<m2g::pb::ObjectType> objs;
		M2_GAME.ForEachObjectWithMainSprite([&objs](const auto objType, MAYBE const auto spriteType) {
			objs.emplace_back(objType);
			return true;
		});
		return objs;
	}

	const UiPanelBlueprint gFillDialog = {
		.name = "FillDialog",
		.w = 32, .h = 18,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.name = "SpriteSelection",
				.x = 1, .y = 1, .w = 30, .h = 14,
				.variant = TextSelectionBlueprint{
					.line_count = 10,
					.allow_multiple_selection = true,
					.onCreate = [](TextSelection &self) {
						TextSelectionBlueprint::Options options;
						for (const auto& sprite : M2_GAME.level_editor_background_sprites) {
							options.emplace_back(pb::enum_name(sprite), I(sprite));
						}
						self.SetOptions(std::move(options));
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 16, .w = 30, .h = 1,
				.variant = TextBlueprint {
					.text = "Fill",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						const auto* spriteSelectionWidget = self.Parent().FindWidget<TextSelection>("SpriteSelection");
						std::vector<m2g::pb::SpriteType> selectedSprites;
						for (const auto& selection : spriteSelectionWidget->GetSelectedOptions()) {
							selectedSprites.emplace_back(static_cast<m2g::pb::SpriteType>(I(selection)));
						}
						return MakeReturnAction(std::move(selectedSprites));
					}
				}
			}
		}
	};

	const UiPanelBlueprint gCoordinateDialog = {
		.name = "CoordinateDialog",
		.w = 23, .h = 13,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.name = "XCoordinate",
				.x = 1, .y = 1, .w = 10, .h = 5,
				.variant = TextInputBlueprint{}
			},
			UiWidgetBlueprint{
				.name = "YCoordinate",
				.x = 1, .y = 7, .w = 10, .h = 5,
				.variant = TextInputBlueprint{}
			},
			UiWidgetBlueprint{
				.x = 12, .y = 1, .w = 10, .h = 5,
				.variant = TextBlueprint{
					.text = "OK",
					.onAction = [](const Text& self) {
						const auto xEntry = self.Parent().FindWidget<TextInput>("XCoordinate")->text_input();
						const auto yEntry = self.Parent().FindWidget<TextInput>("YCoordinate")->text_input();
						const auto xFloat = ToFloat(xEntry);
						const auto yFloat = ToFloat(yEntry);
						if (xFloat && yFloat) {
							return MakeReturnAction(VecF{*xFloat, *yFloat});
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 12, .y = 7, .w = 10, .h = 5,
				.variant = TextBlueprint{
					.text = "Cancel",
					.onAction = [](const Text&) {
						return MakeReturnAction();
					}
				}
			}
		}
	};

	const UiPanelBlueprint gObjectTypeSelectionDialog = {
		.name = "ObjectTypeSelectionDialog",
		.w = 32, .h = 13,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.name = "ObjectTypeSelection",
				.x = 1, .y = 1, .w = 30, .h = 5,
				.variant = TextSelectionBlueprint{
					.onCreate = [](TextSelection& self) {
						const auto objectTypes = ObjectTypesWithMainSprite();
						self.SetOptions(ToTextSelectionOptions(objectTypes.begin(), objectTypes.end()));
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 7, .w = 9, .h = 5,
				.variant = TextBlueprint{
					.text = "Cancel",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 11, .y = 7, .w = 20, .h = 5,
				.variant = TextBlueprint{
					.text = "OK",
					.onAction = [](const Text& self) -> UiAction {
						const auto selectedObjectType = static_cast<m2g::pb::ObjectType>(
								I(self.Parent().FindWidget<TextSelection>("ObjectTypeSelection")->GetSelectedOptions()[0]));
						return MakeReturnAction(selectedObjectType);
					}
				}
			}
		}
	};

	const UiPanelBlueprint gAngleDialog = {
		.w = 23, .h = 17,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1, .y = 1, .w = 10, .h = 3,
				.variant = TextBlueprint{ .text = "Angle:" }
			},
			UiWidgetBlueprint{
				.name = "DegreeInput",
				.x = 12, .y = 1, .w = 10, .h = 3,
				.variant = IntegerSelectionBlueprint{
					.min_value = 0, .max_value = 360
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 5, .w = 10, .h = 3,
				.variant = TextBlueprint{ .text = "Piece Count:" }
			},
			UiWidgetBlueprint{
				.name = "PieceCountInput",
				.x = 12, .y = 5, .w = 10, .h = 3,
				.variant = IntegerSelectionBlueprint{
					.min_value = 1, .max_value = 1000,
					.initial_value = 1
				}
			},
			UiWidgetBlueprint{
				.name = "DrawTowardsRightCheckbox",
				.x = 1, .y = 9, .w = 21, .h = 3,
				.variant = CheckboxWithTextBlueprint{
					.text = "Draw Towards Right"
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 13, .w = 10, .h = 3,
				.variant = TextBlueprint{
					.text = "Cancel",
					.onAction = [](const auto&) {
						return MakeReturnAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 12, .y = 13, .w = 10, .h = 3,
				.variant = TextBlueprint{
					.text = "OK",
					.onAction = [](const Text& self) {
						const auto angle = self.Parent().FindWidget<IntegerSelection>("DegreeInput")->value();
						const auto pieceCount = self.Parent().FindWidget<IntegerSelection>("PieceCountInput")->value();
						const auto drawTowardsRight = self.Parent().FindWidget<CheckboxWithText>("DrawTowardsRightCheckbox")->GetState();
						return MakeReturnAction<level_editor::ArcDescription>(level_editor::ArcDescription{angle, pieceCount, drawTowardsRight});
					}
				}
			}
		}
	};

	UiPanelBlueprint CreateDistinctIntegerSelectionDialog(const int min, const int max) {
		return UiPanelBlueprint{
			.w = 23, .h = 17,
			.background_color = {0, 0, 0, 255},
			.widgets = {
				UiWidgetBlueprint{
					.name = "First",
					.x = 1, .y = 1, .w = 10, .h = 3,
					.variant = IntegerSelectionBlueprint{
						.min_value = min, .max_value = max
					}
				},
				UiWidgetBlueprint{
					.name = "Second",
					.x = 12, .y = 1, .w = 10, .h = 3,
					.variant = IntegerSelectionBlueprint{
						.min_value = min, .max_value = max
					}
				},
				UiWidgetBlueprint{
					.x = 1, .y = 5, .w = 10, .h = 3,
					.variant = TextBlueprint{ .text = "Radius:" }
				},
				UiWidgetBlueprint{
					.name = "Radius",
					.x = 12, .y = 5, .w = 10, .h = 3,
					.variant = TextInputBlueprint{}
				},
				UiWidgetBlueprint{
					.x = 1, .y = 9, .w = 10, .h = 3,
					.variant = TextBlueprint{ .text = "Piece Count:" }
				},
				UiWidgetBlueprint{
					.name = "PieceCountInput",
					.x = 12, .y = 9, .w = 10, .h = 3,
					.variant = IntegerSelectionBlueprint{
						.min_value = 1, .max_value = 1000,
						.initial_value = 1
					}
				},
				UiWidgetBlueprint{
					.x = 1, .y = 13, .w = 10, .h = 3,
					.variant = TextBlueprint{
						.text = "Cancel",
						.onAction = [](const auto&) {
							return MakeReturnAction();
						}
					}
				},
				UiWidgetBlueprint{
					.x = 12, .y = 13, .w = 10, .h = 3,
					.variant = TextBlueprint{
						.text = "OK",
						.onAction = [](const Text& self) {
							const auto first = self.Parent().FindWidget<IntegerSelection>("First")->value();
							const auto second = self.Parent().FindWidget<IntegerSelection>("Second")->value();
							if (first == second) {
								return MakeContinueAction();
							}
							const auto radius = stof(self.Parent().FindWidget<TextInput>("Radius")->text_input());
							const auto pieceCount = self.Parent().FindWidget<IntegerSelection>("PieceCountInput")->value();
							return MakeReturnAction<level_editor::TangentDescription>(level_editor::TangentDescription{first, second, radius, pieceCount});
						}
					}
				}
			}
		};
	}

	const UiPanelBlueprint gDisplayOptionsDialog = {
		.name = "DisplayOptionsDialog",
		.w = 12, .h = 40,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1, .y = 1, .w = 10, .h = 3,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = "Physics"
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 5, .w = 10, .h = 30,
				.variant = TextSelectionBlueprint{
					.line_count = 10,
					.allow_multiple_selection = true,
					.show_scroll_bar = true,
					.onCreate = [](TextSelection& self) {
						const auto objectTypes = ObjectTypesWithMainSprite();
						auto selectionOptions = ToTextSelectionOptions(objectTypes.begin(), objectTypes.end());
						const auto& objectSet = std::get<level_editor::State>(M2_LEVEL.stateVariant).physicsObjectsToDraw;
						for (auto& option : selectionOptions) {
							if (objectSet.contains(static_cast<m2g::pb::ObjectType>(I(option.return_value)))) {
								option.initiallySelected = true;
							}
						}
						self.SetOptions(selectionOptions);
					},
					.onAction = [](const TextSelection& self) -> UiAction {
						auto& objectSet = std::get<level_editor::State>(M2_LEVEL.stateVariant).physicsObjectsToDraw;
						objectSet.clear();
						std::ranges::transform(self.GetSelectedOptions(), std::inserter(objectSet, objectSet.begin()),
							[](const auto& retval) { return static_cast<m2g::pb::ObjectType>(I(retval)); });
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 36, .w = 10, .h = 3,
				.variant = TextBlueprint{
					.text = "OK",
					.onAction = [](const Text&) -> UiAction {
						return MakeReturnAction();
					}
				}
			},
		}
	};

	const UiPanelBlueprint gPaintBgRightHud = {
		.name = "PaintBgRightHud",
		.w = 19, .h = 72,
		.background_color = {25, 25, 25, 255},
		.onDestroy = [] {
			// If level isn't destroyed yet
			if (std::holds_alternative<level_editor::State>(M2_LEVEL.stateVariant)) {
				// Delete ghost
				if (const auto& levelEditorState = std::get<level_editor::State>(M2_LEVEL.stateVariant); levelEditorState.ghostId) {
					M2_LEVEL.deferredActions.push(CreateObjectDeleter(levelEditorState.ghostId));
				}
			}
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2, .y = 2, .w = 15, .h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Paint Bg"
				}
			},
			UiWidgetBlueprint{
				.name = "SpriteTypeSelection",
				.x = 2, .y = 7, .w = 15, .h = 48,
				.variant = TextSelectionBlueprint{
					.line_count = 12,
					.onCreate = [](TextSelection& self) {
						// Fill sprite type selector with background sprite types
						TextSelectionBlueprint::Options options;
						for (const auto &spriteType: M2_GAME.level_editor_background_sprites) {
							options.emplace_back(SpriteType_Name(spriteType), I(spriteType));
						}
						self.SetOptions(std::move(options));
						self.SetUniqueSelectionIndex(0);
					},
					.onAction = [](const TextSelection& self) {
						// Create ghost
						auto& levelEditorState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
						if (levelEditorState.ghostId) {
							M2_LEVEL.deferredActions.push(CreateObjectDeleter(levelEditorState.ghostId));
						}
						if (const auto selections = self.GetSelectedOptions(); not selections.empty()) {
							levelEditorState.ghostId = obj::create_ghost(
									static_cast<m2g::pb::SpriteType>(I(selections[0])), 1);
						}
						return MakeContinueAction();
					}
				}
			}
		}
	};

	const UiPanelBlueprint gSampleBgRightHud = {
		.name = "SampleBgRightHud",
		.w = 19, .h = 72,
		.background_color = {25, 25, 25, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2, .y = 2, .w = 15, .h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Sample Bg"
				}
			}
		}
	};

	const UiPanelBlueprint gSelectBgRightHud = {
		.name = "SelectBgRightHud",
		.w = 19, .h = 72,
		.background_color = {25, 25, 25, 255},
		.onCreate = [](MAYBE UiPanel& self) {
			M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());
		},
		.onDestroy = [] {
			M2_LEVEL.DisablePrimarySelection();
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2, .y = 2, .w = 15, .h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Select Bg"
				}
			},
			UiWidgetBlueprint{
				.x = 2, .y = 7, .w = 15, .h = 4,
				.variant = TextBlueprint{
					.text = "Copy",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						if (const auto integerSelection = M2_LEVEL.PrimarySelection()->IntegerSelectionRectM()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).CopyBackground(*integerSelection);
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2, .y = 12, .w = 15, .h = 4,
				.variant = TextBlueprint{
					.text = "Paste",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						if (const auto integerSelection = M2_LEVEL.PrimarySelection()->IntegerSelectionRectM()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).PasteBackground(integerSelection->top_left());
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2, .y = 17, .w = 15, .h = 4,
				.variant = TextBlueprint{
					.text = "Erase",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						if (const auto integerSelection = M2_LEVEL.PrimarySelection()->IntegerSelectionRectM()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).EraseBackground(*integerSelection);
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2, .y = 22, .w = 15, .h = 4,
				.variant = TextBlueprint{
					.text = "Fill",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						if (const auto* selection = M2_LEVEL.PrimarySelection(); selection->IsComplete()) {
							const auto area = selection->IntegerSelectionRectM();
							const auto action = UiPanel::create_and_run_blocking(&gFillDialog, RectF{0.2f, 0.1f, 0.6f, 0.8f});
							(void) action.IfReturn<std::vector<m2g::pb::SpriteType>>([&](const auto& selectedSprites) {
								std::get<level_editor::State>(M2_LEVEL.stateVariant).RandomFillBackground(*area, selectedSprites);
							});
						}
						return MakeContinueAction();
					}
				}
			}
		}
	};

	const UiPanelBlueprint gPlaceFgRightHud = {
		.name = "PlaceFgRightHud",
		.w = 19, .h = 72,
		.background_color = {25, 25, 25, 255},
		.onDestroy = [] {
			// If level isn't destroyed yet
			if (std::holds_alternative<level_editor::State>(M2_LEVEL.stateVariant)) {
				// Delete ghost
				if (const auto& levelEditorState = std::get<level_editor::State>(M2_LEVEL.stateVariant); levelEditorState.ghostId) {
					M2_LEVEL.deferredActions.push(CreateObjectDeleter(levelEditorState.ghostId));
				}
			}
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2, .y = 2, .w = 15, .h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Place Fg"
				}
			},
			UiWidgetBlueprint{
				.name = "OrientationInput",
				.x = 2, .y = 7, .w = 15, .h = 4,
				.variant = IntegerSelectionBlueprint{
					.min_value = 0,
					.max_value = 359,
					.initial_value = 0
				}
			},
			UiWidgetBlueprint{
				.name = "ObjectTypeSelection",
				.x = 2, .y = 12, .w = 15, .h = 40,
				.variant = TextSelectionBlueprint{
					.line_count = 20,
					.onCreate = [](TextSelection& self) {
						const auto objectTypes = ObjectTypesWithMainSprite();
						self.SetOptions(ToTextSelectionOptions(objectTypes.begin(), objectTypes.end()));
						self.SetUniqueSelectionIndex(0);
					},
					.onAction = [](const TextSelection& self) {
						// Delete previous ghost
						auto& levelEditorState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
						if (levelEditorState.ghostId) {
							M2_LEVEL.deferredActions.push(CreateObjectDeleter(levelEditorState.ghostId));
						}
						// Create ghost
						if (const auto selections = self.GetSelectedOptions(); not selections.empty()) {
							const auto snapToGrid = M2_LEVEL.LeftHud()->FindWidget<CheckboxWithText>("SnapToGridCheckbox")->GetState();
							const auto splitCount = M2_LEVEL.LeftHud()->FindWidget<IntegerSelection>("CellSplitCount")->value();
							levelEditorState.ghostId = obj::create_ghost(*M2_GAME.GetMainSpriteOfObject(static_cast<m2g::pb::ObjectType>(I(selections[0]))), snapToGrid ? splitCount : 0);
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.name = "GroupTypeSelection",
				.x = 2, .y = 53, .w = 15, .h = 4,
				.variant = TextSelectionBlueprint{
					.line_count = 0,
					.onCreate = [](TextSelection& self) {
						// Fill group type selector
						TextSelectionBlueprint::Options options;
						for (int e = 0; e < pb::enum_value_count<m2g::pb::GroupType>(); ++e) {
							options.emplace_back(pb::enum_name<m2g::pb::GroupType>(e), pb::enum_value<m2g::pb::GroupType>(e));
						}
						self.SetOptions(std::move(options));
					}
				}
			},
			UiWidgetBlueprint{
				.name = "GroupInstanceSelection",
				.x = 2, .y = 58, .w = 15, .h = 4,
				.variant = IntegerSelectionBlueprint{
					.min_value = 0,
					.max_value = 999,
					.initial_value = 0
				}
			},
			UiWidgetBlueprint{
				.name = "PlaceButton",
				.x = 2, .y = 63, .w = 15, .h = 4,
				.variant = TextBlueprint{
					.text = "Place",
					.onAction = [](const Text&) -> UiAction {
						(void) UiPanel::create_and_run_blocking(&gCoordinateDialog, RectF{0.25f, 0.4f, 0.5f, 0.3f})
								.IfReturn<VecF>([](const auto& vec) {
									std::get<level_editor::State>(M2_LEVEL.stateVariant).HandleMouseSecondaryButton(vec);
								});
						return MakeContinueAction();
					}
				}
			}
		}
	};

	const UiPanelBlueprint gSampleFgRightHud = {
		.name = "SampleFgRightHud",
		.w = 19, .h = 72,
		.background_color = {25, 25, 25, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2, .y = 2, .w = 15, .h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Sample Fg"
				}
			}
		}
	};

	const UiPanelBlueprint gSelectFgRightHud = {
		.name = "SelectFgRightHud",
		.w = 19, .h = 72,
		.background_color = {25, 25, 25, 255},
		.onCreate = [](MAYBE UiPanel& self) {
			M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());
		},
		.onDestroy = [] {
			M2_LEVEL.DisablePrimarySelection();
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2, .y = 2, .w = 15, .h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Select Fg"
				}
			},
			UiWidgetBlueprint{
				.x = 2, .y = 7, .w = 15, .h = 4,
				.variant = TextBlueprint{
					.text = "Copy",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						if (M2_LEVEL.PrimarySelection()->IsComplete()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).CopyForeground();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2, .y = 12, .w = 15, .h = 4,
				.variant = TextBlueprint{
					.text = "Paste",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						if (M2_LEVEL.PrimarySelection()->IsComplete()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).PasteForeground();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2, .y = 17, .w = 15, .h = 4,
				.variant = TextBlueprint{
					.text = "Remove",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						if (M2_LEVEL.PrimarySelection()->IsComplete()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).RemoveForegroundObject();
						}
						return MakeContinueAction();
					}
				}
			}
		}
	};

	const UiPanelBlueprint gDrawFgRightHud = {
		.name = "DrawFgRightHud",
		.w = 19, .h = 72,
		.background_color = {25, 25, 25, 255},
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
				.name = "FixtureSelection",
				.x = 1, .y = 1, .w = 17, .h = 42,
				.variant = TextSelectionBlueprint{
					.line_count = 21,
					.show_scroll_bar = true,
					.onHover = [](const TextSelection& self, const std::optional<int> indexUnderMouse) {
						if (indexUnderMouse) {
							const auto& leditState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
							const auto& drawFgState = dynamic_cast<level_editor::DrawFgRightHudState&>(*self.Parent().state);
							const auto fixtureName = leditState.GetSpritePb(drawFgState.SelectedObjectMainSpriteType()).regular().fixtures(*indexUnderMouse).name();
							M2_LEVEL.SetMouseHoverUiPanel(std::make_unique<UiPanelBlueprint>(CreateTextTooltipBlueprint(fixtureName)), gTextTooltipRatio);
						}
					},
					.offHover = [](MAYBE TextSelection& self) {
						M2_LEVEL.RemoveMouseHoverUiPanel();
					},
					.onUpdate = [](TextSelection& self) -> UiAction {
						const auto& leditState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
						if (const auto& state = dynamic_cast<level_editor::DrawFgRightHudState&>(*self.Parent().state);
								I(self.GetOptions().size()) != leditState.GetSpritePb(state.SelectedObjectMainSpriteType()).regular().fixtures_size()) {
							TextSelectionBlueprint::Options options;
							std::ranges::transform(leditState.GetSpritePb(state.SelectedObjectMainSpriteType()).regular().fixtures(), std::back_inserter(options),
								[](const auto& fixture) -> TextSelectionBlueprint::Option {
									auto line = sheet_editor::gFixtureTypeNames.at(fixture.fixture_type_case()) + "(" + fixture.name() + ")";
									return {.text = std::move(line)};
								});
							self.SetOptions(std::move(options));
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 44, .w = 17, .h = 3,
				.variant = TextBlueprint{
					.text = "Add Chain",
					.onAction = [](const Text& self) -> UiAction {
						auto* fixtureSelectionWidget = self.Parent().FindWidget<TextSelection>("FixtureSelection");
						const auto selectedIndexes = fixtureSelectionWidget->GetSelectedIndexes();
						const auto selectedIndex = selectedIndexes.empty() ? -1 : selectedIndexes[0];
						auto& leditState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
						const auto& state = dynamic_cast<level_editor::DrawFgRightHudState&>(*self.Parent().state);
						const auto newIndex = leditState.AddChain(state.SelectedObjectMainSpriteType(), selectedIndex);
						fixtureSelectionWidget->UpdateContents();
						fixtureSelectionWidget->SetUniqueSelectionIndex(newIndex);
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 48, .w = 17, .h = 3,
				.variant = TextBlueprint{
					.text = "Remove Fixture",
					.onAction = [](const Text& self) -> UiAction {
						const auto* fixtureSelectionWidget = self.Parent().FindWidget<TextSelection>("FixtureSelection");
						if (const auto selectedIndexes = fixtureSelectionWidget->GetSelectedIndexes(); not selectedIndexes.empty()) {
							const auto selectedIndex = selectedIndexes[0];
							auto& leditState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
							const auto& state = dynamic_cast<level_editor::DrawFgRightHudState&>(*self.Parent().state);
							leditState.RemoveFixture(state.SelectedObjectMainSpriteType(), selectedIndex);
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 52, .w = 17, .h = 3,
				.variant = TextBlueprint{
					.text = "Store Point",
					.onAction = [](const Text& self) -> UiAction {
						if (const auto* selection = M2_LEVEL.SecondarySelection(); selection->IsComplete()) {
							const auto* fixtureSelectionWidget = self.Parent().FindWidget<TextSelection>("FixtureSelection");
							if (const auto selectedIndexes = fixtureSelectionWidget->GetSelectedIndexes(); not selectedIndexes.empty()) {
								const auto selectedIndex = selectedIndexes[0];
								const auto& state = dynamic_cast<level_editor::DrawFgRightHudState&>(*self.Parent().state);
								if (const auto fixtureType = std::get<level_editor::State>(M2_LEVEL.stateVariant).GetSpriteFixtureTypes(state.SelectedObjectMainSpriteType())[selectedIndex];
										fixtureType == pb::Fixture::FixtureTypeCase::kChain) {
									std::get<level_editor::State>(M2_LEVEL.stateVariant).StoreWorldPoint(selectedIndex, selection->SelectionsM()->first);
								}
							}
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 56, .w = 17, .h = 3,
				.variant = TextBlueprint{
					.text = "Store Arc",
					.onAction = [](const Text& self) -> UiAction {
						if (const auto* selection = M2_LEVEL.SecondarySelection(); selection->IsComplete()) {
							const auto point = selection->SelectionsM()->first;

							const auto* fixtureSelectionWidget = self.Parent().FindWidget<TextSelection>("FixtureSelection");
							if (const auto selectedIndexes = fixtureSelectionWidget->GetSelectedIndexes(); not selectedIndexes.empty()) {
								const auto selectedIndex = selectedIndexes[0];

								const auto& state = dynamic_cast<level_editor::DrawFgRightHudState&>(*self.Parent().state);
								if (const auto fixtureType = std::get<level_editor::State>(M2_LEVEL.stateVariant).GetSpriteFixtureTypes(state.SelectedObjectMainSpriteType())[selectedIndex];
										fixtureType == pb::Fixture::FixtureTypeCase::kChain) {
									UiPanel::create_and_run_blocking(&gAngleDialog, RectF{0.27f, 0.37f, 0.46f, 0.26f})
										.IfReturn<level_editor::ArcDescription>([=](const auto& arcDesc) {
											std::get<level_editor::State>(M2_LEVEL.stateVariant).StoreArc(selectedIndex, point, arcDesc);
										});
								}
							}
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 60, .w = 17, .h = 3,
				.variant = TextBlueprint{
					.text = "Store Tangent",
					.onAction = [](const Text& self) -> UiAction {
						const auto* fixtureSelectionWidget = self.Parent().FindWidget<TextSelection>("FixtureSelection");
						if (const auto selectedIndexes = fixtureSelectionWidget->GetSelectedIndexes(); not selectedIndexes.empty()) {
							const auto selectedIndex = selectedIndexes[0];

							const auto& leditState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
							const auto& drawFgState = dynamic_cast<level_editor::DrawFgRightHudState&>(*self.Parent().state);
							if (const auto fixtureType = leditState.GetSpriteFixtureTypes(drawFgState.SelectedObjectMainSpriteType())[selectedIndex];
									fixtureType == pb::Fixture::FixtureTypeCase::kChain) {

								// There are same number of line segments as number of points
								if (const auto pointCount = leditState.GetSpritePb(drawFgState.SelectedObjectMainSpriteType()).regular().fixtures(selectedIndex).chain().points_size(); 1 < pointCount) {
									UiPanel::create_and_run_blocking(
										std::make_unique<UiPanelBlueprint>(CreateDistinctIntegerSelectionDialog(0, pointCount - 1)), RectF{0.27f, 0.37f, 0.46f, 0.26f})
											.IfReturn<level_editor::TangentDescription>([=](const auto& tangent) {
												std::get<level_editor::State>(M2_LEVEL.stateVariant).StoreTangent(selectedIndex, tangent);
											});
								}
							}
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 64, .w = 17, .h = 3,
				.variant = TextBlueprint{
					.text = "Undo Point",
					.onAction = [](const Text& self) -> UiAction {
						const auto* fixtureSelectionWidget = self.Parent().FindWidget<TextSelection>("FixtureSelection");
						if (const auto selectedIndexes = fixtureSelectionWidget->GetSelectedIndexes(); not selectedIndexes.empty()) {
							const auto selectedIndex = selectedIndexes[0];
							const auto& state = dynamic_cast<level_editor::DrawFgRightHudState&>(*self.Parent().state);
							if (const auto fixtureType = std::get<level_editor::State>(M2_LEVEL.stateVariant).GetSpriteFixtureTypes(state.SelectedObjectMainSpriteType())[selectedIndex];
									fixtureType == pb::Fixture::FixtureTypeCase::kChain) {
								std::get<level_editor::State>(M2_LEVEL.stateVariant).UndoPoint(selectedIndex);
							}
						}
						return MakeContinueAction();
					}
				}
			},
		}
	};
}

const UiPanelBlueprint level_editor::gLeftHudBlueprint = {
	.name = "LeftHud",
    .w = 19, .h = 72,
    .background_color = {25, 25, 25, 255},
    .widgets = *MakeVerticalLayout(19, 72, 1, {
        UiWidgetBlueprint{
        	.name = "PaintBgButton",
            .h = 3,
        	.variant = TextBlueprint{
        		.text = "Paint Bg",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gPaintBgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
        UiWidgetBlueprint{
            .h = 3,
        	.variant = TextBlueprint{
        		.text = "Sample Bg",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gSampleBgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
        UiWidgetBlueprint{
            .h = 3,
        	.variant = TextBlueprint{
        		.text = "Select Bg",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gSelectBgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
        UiWidgetBlueprint{
        	.name = "PlaceFgButton",
            .h = 3,
        	.variant = TextBlueprint{
        		.text = "Place Fg",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gPlaceFgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
        UiWidgetBlueprint{
            .h = 3,
            .variant = TextBlueprint{
				.text = "Sample Fg",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gSampleFgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
				}
			}
		},
        UiWidgetBlueprint{
            .h = 3,
        	.variant = TextBlueprint{
        		.text = "Select Fg",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gSelectFgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
		UiWidgetBlueprint{
			.h = 3,
			.variant = TextBlueprint{
				.text = "Draw Fg",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					(void) UiPanel::create_and_run_blocking(&gObjectTypeSelectionDialog, RectF{0.25f, 0.4f, 0.5f, 0.3f})
						.IfReturn<m2g::pb::ObjectType>([](const auto& typ) {
							if (not M2_GAME.GetMainSpriteOfObject(typ)) {
								M2_LEVEL.ShowMessage("Selected object doesn't have a main sprite", 8.0f);
								return;
							}
							if (std::get<State>(M2_LEVEL.stateVariant).GetForegroundObjectsOfType(typ).size() != 1) {
								M2_LEVEL.ShowMessage("Only one instance of selected object type must be present in the level", 8.0f);
								return;
							}
							M2_LEVEL.ReplaceRightHud(&gDrawFgRightHud, M2_GAME.Dimensions().RightHud());
							M2_LEVEL.RightHud()->state = std::make_unique<DrawFgRightHudState>(typ);
						});
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.name = "CancelButton",
			.h = 3,
			.variant = TextBlueprint{
				.text = "Cancel",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gRightHudBlueprint, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
				}
			}
		},
    	DynamicSpacer{},
		UiWidgetBlueprint{
			.name = "BackgroundLayerSelection",
			.h = 3,
			.variant = TextSelectionBlueprint{
				.options = {
					TextSelectionBlueprint::Option{.text = "B0-Front", .return_value = I(BackgroundLayer::B0)},
					TextSelectionBlueprint::Option{.text = "B1", .return_value = I(BackgroundLayer::B1)},
					TextSelectionBlueprint::Option{.text = "B2", .return_value = I(BackgroundLayer::B2)},
					TextSelectionBlueprint::Option{.text = "B3-Back", .return_value = I(BackgroundLayer::B3)}
				},
				.line_count = 0,
				.allow_multiple_selection = false,
				.show_scroll_bar = false,
			}
		},
		UiWidgetBlueprint{
			.h = 3,
			.variant = TextBlueprint{
				.text = "Display Options",
				.onAction = [](MAYBE const Text& self) -> UiAction {
					UiPanel::create_and_run_blocking(&gDisplayOptionsDialog, RectF{0.3f, 0.1f, 0.4f, 0.8f});
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.name = "SnapToGridCheckbox",
            .h = 3,
			.variant = CheckboxWithTextBlueprint{
				.text = "Snap",
				.initial_state = false,
				.onAction = [](const CheckboxWithText& self) {
					// Press the cancel button if PlaceFgRightHud is active because the Ghost needs to be recreated
					if (M2_LEVEL.RightHud()->Name() == "PlaceFgRightHud") {
						self.Parent().FindWidget<Text>("CancelButton")->trigger_action();
					}
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.name = "ShowGridCheckbox",
			.h = 3,
			.variant = CheckboxWithTextBlueprint{
				.text = "Grid",
				.initial_state = false,
				.onAction = [](const CheckboxWithText& self) -> UiAction {
					self.Parent().FindWidget<IntegerSelection>("CellSplitCount")->enabled = self.GetState();
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.name = "CellSplitCount",
			.initially_enabled = false,
			.h = 3,
			.variant = IntegerSelectionBlueprint{
				.min_value = 1,
				.max_value = 100,
				.initial_value = 1
			}
		},
    	DynamicSpacer{},
        UiWidgetBlueprint{
            .h = 3,
            .variant = TextBlueprint{
            	.text = "Save",
            	.onAction = [](MAYBE const Text& self) -> UiAction {
            		if (auto success = std::get<State>(M2_LEVEL.stateVariant).Save(); not success) {
            			M2_LEVEL.ShowMessage(success.error(), 8.0f);
            		}
            		return MakeContinueAction();
            	}
            }
        },
        UiWidgetBlueprint{
        	.h = 2,
        	.border_width = 0,
        	.variant = TextBlueprint{
        		.text = "0:0",
        		.onUpdate = [](MAYBE Text& self) {
        			const auto mouse_position = M2_GAME.MousePositionWorldM().iround();
        			self.set_text(ToString(mouse_position.x) + ':' + m2::ToString(mouse_position.y));
        			return MakeContinueAction();
        		}
        	}
        }
    })
};

const UiPanelBlueprint level_editor::gRightHudBlueprint = {
		.name = "RightHud",
	    .w = 19, .h = 72,
		.background_color = {25, 25, 25, 255}};
