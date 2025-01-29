#include <m2/level_editor/Ui.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/IntegerInput.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/CheckboxWithTextBlueprint.h>
#include <m2/protobuf/Detail.h>
#include <m2/game/object/Ghost.h>
#include <m2/Game.h>
#include <m2/game/Selection.h>
#include <m2/ui/widget/CheckboxWithText.h>

using namespace m2;
using namespace m2::widget;

namespace {
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
					.on_create = [](TextSelection &self) {
						TextSelectionBlueprint::Options options;
						for (const auto& sprite : M2_GAME.level_editor_background_sprites) {
							options.emplace_back(pb::enum_name(sprite), I(sprite));
						}
						self.set_options(std::move(options));
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 16, .w = 30, .h = 1,
				.variant = TextBlueprint {
					.text = "Fill",
					.on_action = [](MAYBE const Text& self) -> UiAction {
						const auto* spriteSelectionWidget = self.parent().find_first_widget_by_name<TextSelection>("SpriteSelection");
						std::vector<m2g::pb::SpriteType> selectedSprites;
						for (const auto& selection : spriteSelectionWidget->selections()) {
							selectedSprites.emplace_back(static_cast<m2g::pb::SpriteType>(std::get<int>(selection)));
						}
						return MakeReturnAction(std::move(selectedSprites));
					}
				}
			}
		}
	};

	const UiPanelBlueprint gPaintBgRightHud = {
		.name = "PaintBgRightHud",
		.w = 19,
		.h = 72,
		.background_color = {25, 25, 25, 255},
		.onDestroy = [] {
			// Delete ghost
			if (const auto& levelEditorState = std::get<level_editor::State>(M2_LEVEL.stateVariant); levelEditorState.ghostId) {
				M2_LEVEL.deferredActions.push(create_object_deleter(levelEditorState.ghostId));
			}
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2,
				.y = 2,
				.w = 15,
				.h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Paint Bg"
				}
			},
			UiWidgetBlueprint{
				.name = "SpriteTypeSelection",
				.x = 2,
				.y = 7,
				.w = 15,
				.h = 48,
				.variant = TextSelectionBlueprint{
					.line_count = 12,
					.on_create = [](TextSelection& self) {
						// Fill sprite type selector with background sprite types
						TextSelectionBlueprint::Options options;
						for (const auto &spriteType: M2_GAME.level_editor_background_sprites) {
							options.emplace_back(SpriteType_Name(spriteType), I(spriteType));
						}
						self.set_options(std::move(options));
						self.set_unique_selection(0);
					},
					.on_action = [](const TextSelection& self) {
						// Create ghost
						auto& levelEditorState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
						if (levelEditorState.ghostId) {
							M2_LEVEL.deferredActions.push(create_object_deleter(levelEditorState.ghostId));
						}
						if (const auto selections = self.selections(); not selections.empty()) {
							levelEditorState.ghostId = obj::create_ghost(
									static_cast<m2g::pb::SpriteType>(std::get<int>(selections[0])), true);
						}
						return MakeContinueAction();
					}
				}
			}
		}
	};

	const UiPanelBlueprint gSampleBgRightHud = {
		.name = "SampleBgRightHud",
		.w = 19,
		.h = 72,
		.background_color = {25, 25, 25, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2,
				.y = 2,
				.w = 15,
				.h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Sample Bg"
				}
			}
		}
	};

	const UiPanelBlueprint gSelectBgRightHud = {
		.name = "SelectBgRightHud",
		.w = 19,
		.h = 72,
		.background_color = {25, 25, 25, 255},
		.on_create = [](MAYBE UiPanel& self) {
			Events::enable_primary_selection(RectI{M2_GAME.Dimensions().Game()});
		},
		.onDestroy = [] {
			Events::disable_primary_selection();
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2,
				.y = 2,
				.w = 15,
				.h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Select Bg"
				}
			},
			UiWidgetBlueprint{
				.x = 2,
				.y = 7,
				.w = 15,
				.h = 4,
				.variant = TextBlueprint{
					.text = "Copy",
					.on_action = [](MAYBE const Text& self) -> UiAction {
						if (const auto area = SelectionResult{M2_GAME.events}.PrimaryIntegerRoundedSelectionRectM()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).CopyBackground(*area);
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2,
				.y = 12,
				.w = 15,
				.h = 4,
				.variant = TextBlueprint{
					.text = "Paste",
					.on_action = [](MAYBE const Text& self) -> UiAction {
						if (const auto area = SelectionResult{M2_GAME.events}.PrimaryIntegerRoundedSelectionRectM()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).PasteBackground(area->top_left());
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2,
				.y = 17,
				.w = 15,
				.h = 4,
				.variant = TextBlueprint{
					.text = "Erase",
					.on_action = [](MAYBE const Text& self) -> UiAction {
						if (const auto area = SelectionResult{M2_GAME.events}.PrimaryIntegerRoundedSelectionRectM()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).EraseBackground(*area);
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2,
				.y = 22,
				.w = 15,
				.h = 4,
				.variant = TextBlueprint{
					.text = "Fill",
					.on_action = [](MAYBE const Text& self) -> UiAction {
						if (const auto selectionResult = SelectionResult{M2_GAME.events}; selectionResult.is_primary_selection_finished()) {
							const auto area = selectionResult.PrimaryIntegerRoundedSelectionRectM();
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
		.w = 19,
		.h = 72,
		.background_color = {25, 25, 25, 255},
		.onDestroy = [] {
			// Delete ghost
			if (const auto& levelEditorState = std::get<level_editor::State>(M2_LEVEL.stateVariant); levelEditorState.ghostId) {
				M2_LEVEL.deferredActions.push(create_object_deleter(levelEditorState.ghostId));
			}
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2,
				.y = 2,
				.w = 15,
				.h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Place Fg"
				}
			},
			UiWidgetBlueprint{
				.name = "ObjectTypeSelection",
				.x = 2,
				.y = 7,
				.w = 15,
				.h = 48,
				.variant = TextSelectionBlueprint{
					.line_count = 12,
					.on_create = [](TextSelection& self) {
						// Fill object type selector with editor-enabled object types
						TextSelectionBlueprint::Options options;
						for (const auto &objType: M2_GAME.object_main_sprites | std::views::keys) {
							options.emplace_back(ObjectType_Name(objType), I(objType));
						}
						self.set_options(std::move(options));
						self.set_unique_selection(0);
					},
					.on_action = [](const TextSelection& self) {
						// Delete previous ghost
						auto& levelEditorState = std::get<level_editor::State>(M2_LEVEL.stateVariant);
						if (levelEditorState.ghostId) {
							M2_LEVEL.deferredActions.push(create_object_deleter(levelEditorState.ghostId));
						}
						// Create ghost
						if (const auto selections = self.selections(); not selections.empty()) {
							const auto snapToGrid = M2_LEVEL.LeftHud()->find_first_widget_by_name<CheckboxWithText>("SnapToGridCheckbox")->state();
							levelEditorState.ghostId = obj::create_ghost(
									M2_GAME.object_main_sprites[static_cast<m2g::pb::ObjectType>(std::get<int>(selections[0]))], snapToGrid);
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.name = "GroupTypeSelection",
				.x = 2,
				.y = 56,
				.w = 15,
				.h = 4,
				.variant = TextSelectionBlueprint{
					.line_count = 0,
					.on_create = [](TextSelection& self) {
						// Fill group type selector
						TextSelectionBlueprint::Options options;
						for (int e = 0; e < pb::enum_value_count<m2g::pb::GroupType>(); ++e) {
							options.emplace_back(pb::enum_name<m2g::pb::GroupType>(e), pb::enum_value<m2g::pb::GroupType>(e));
						}
						self.set_options(std::move(options));
					}
				}
			},
			UiWidgetBlueprint{
				.name = "GroupInstanceSelection",
				.x = 2,
				.y = 61,
				.w = 15,
				.h = 4,
				.variant = IntegerInputBlueprint{
					.min_value = 0,
					.max_value = 999,
					.initial_value = 0
				}
			}
		}
	};

	const UiPanelBlueprint gSampleFgRightHud = {
		.name = "SampleFgRightHud",
		.w = 19,
		.h = 72,
		.background_color = {25, 25, 25, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2,
				.y = 2,
				.w = 15,
				.h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Sample Fg"
				}
			}
		}
	};

	const UiPanelBlueprint gSelectFgRightHud = {
		.name = "SelectFgRightHud",
		.w = 19,
		.h = 72,
		.background_color = {25, 25, 25, 255},
		.on_create = [](MAYBE UiPanel& self) {
			Events::enable_primary_selection(RectI{M2_GAME.Dimensions().Game()});
		},
		.onDestroy = [] {
			Events::disable_primary_selection();
		},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2,
				.y = 2,
				.w = 15,
				.h = 4,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Select Fg"
				}
			},
			UiWidgetBlueprint{
				.x = 2,
				.y = 7,
				.w = 15,
				.h = 4,
				.variant = TextBlueprint{
					.text = "Copy",
					.on_action = [](MAYBE const Text& self) -> UiAction {
						if (SelectionResult{M2_GAME.events}.is_primary_selection_finished()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).CopyForeground();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2,
				.y = 12,
				.w = 15,
				.h = 4,
				.variant = TextBlueprint{
					.text = "Paste",
					.on_action = [](MAYBE const Text& self) -> UiAction {
						if (SelectionResult{M2_GAME.events}.is_primary_selection_finished()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).PasteForeground();
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2,
				.y = 17,
				.w = 15,
				.h = 4,
				.variant = TextBlueprint{
					.text = "Remove",
					.on_action = [](MAYBE const Text& self) -> UiAction {
						if (SelectionResult{M2_GAME.events}.is_primary_selection_finished()) {
							std::get<level_editor::State>(M2_LEVEL.stateVariant).RemoveForegroundObject();
						}
						return MakeContinueAction();
					}
				}
			}
		}
	};
}

const UiPanelBlueprint level_editor::gLeftHudBlueprint = {
	.name = "LeftHud",
    .w = 19,
    .h = 72,
    .background_color = {25, 25, 25, 255},
    .widgets = {
        UiWidgetBlueprint{
        	.name = "PaintBgButton",
            .x = 2,
            .y = 2,
            .w = 15,
            .h = 4,
        	.variant = TextBlueprint{
        		.text = "Paint Bg",
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gPaintBgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
        UiWidgetBlueprint{
            .x = 2,
            .y = 7,
            .w = 15,
            .h = 4,
        	.variant = TextBlueprint{
        		.text = "Sample Bg",
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gSampleBgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
        UiWidgetBlueprint{
            .x = 2,
            .y = 12,
            .w = 15,
            .h = 4,
        	.variant = TextBlueprint{
        		.text = "Select Bg",
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gSelectBgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
        UiWidgetBlueprint{
        	.name = "PlaceFgButton",
            .x = 2,
            .y = 17,
            .w = 15,
            .h = 4,
        	.variant = TextBlueprint{
        		.text = "Place Fg",
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gPlaceFgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
        UiWidgetBlueprint{
            .x = 2,
            .y = 22,
            .w = 15,
            .h = 4,
            .variant = TextBlueprint{
				.text = "Sample Fg",
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gSampleFgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
				}
			}
		},
        UiWidgetBlueprint{
            .x = 2,
            .y = 27,
            .w = 15,
            .h = 4,
        	.variant = TextBlueprint{
        		.text = "Select Fg",
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gSelectFgRightHud, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
        		}
        	}
        },
		UiWidgetBlueprint{
			.name = "CancelButton",
			.x = 2,
			.y = 32,
			.w = 15,
			.h = 4,
			.variant = TextBlueprint{
				.text = "Cancel",
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ReplaceRightHud(&gRightHudBlueprint, M2_GAME.Dimensions().RightHud());
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.name = "BackgroundLayerSelection",
			.x = 2,
			.y = 50,
			.w = 15,
			.h = 4,
			.variant = TextSelectionBlueprint{
				.options = {
					TextSelectionBlueprint::Option{.text = "L0-Front", .return_value = I(BackgroundLayer::L0)},
					TextSelectionBlueprint::Option{.text = "L1", .return_value = I(BackgroundLayer::L1)},
					TextSelectionBlueprint::Option{.text = "L2", .return_value = I(BackgroundLayer::L2)},
					TextSelectionBlueprint::Option{.text = "L3-Back", .return_value = I(BackgroundLayer::L3)}
				},
				.line_count = 0,
				.allow_multiple_selection = false,
				.show_scroll_bar = false,
			}
		},
		UiWidgetBlueprint{
			.name = "SnapToGridCheckbox",
            .x = 2,
            .y = 55,
            .w = 15,
            .h = 4,
			.variant = CheckboxWithTextBlueprint{
				.text = "Snap",
				.initial_state = false,
				.on_action = [](const CheckboxWithText& self) {
					// Press the cancel button if PlaceFgRightHud is active because the Ghost needs to be recreated
					if (M2_LEVEL.RightHud()->Name() == "PlaceFgRightHud") {
						self.parent().find_first_widget_by_name<Text>("CancelButton")->trigger_action();
					}
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.name = "ShowGridCheckbox",
			.x = 2,
			.y = 60,
			.w = 15,
			.h = 4,
			.variant = CheckboxWithTextBlueprint{
				.text = "Grid",
				.initial_state = false,
			}
		},
        UiWidgetBlueprint{
            .x = 2,
            .y = 65,
            .w = 15,
            .h = 4,
            .variant = TextBlueprint{
            	.text = "Save",
            	.on_action = [](MAYBE const Text& self) -> UiAction {
            		// TODO confirm
            		if (auto success = std::get<State>(M2_LEVEL.stateVariant).Save(); not success) {
            			M2_LEVEL.ShowMessage(success.error(), 8.0f);
            		}
            		return MakeContinueAction();
            	}
            }
        },
        UiWidgetBlueprint{
        	.x = 0,
        	.y = 70,
        	.w = 19,
        	.h = 2,
        	.border_width = 0,
        	.variant = TextBlueprint{
        		.text = "0:0",
        		.on_update = [](MAYBE Text& self) {
        			const auto mouse_position = M2_GAME.MousePositionWorldM().iround();
        			self.set_text(ToString(mouse_position.x) + ':' + m2::ToString(mouse_position.y));
        			return MakeContinueAction();
        		}
        	}
        }
    }
};

const UiPanelBlueprint level_editor::gRightHudBlueprint = {
		.name = "RightHud",
	    .w = 19, .h = 72,
		.background_color = {25, 25, 25, 255}};
