#include <m2/Game.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/game/Selection.h>

using namespace m2;

namespace {
	std::optional<m2g::pb::SpriteType> SelectedSprite() {
		if (const auto selections = M2_LEVEL.LeftHud()->FindWidget<widget::TextSelection>("SpriteTypeSelection")->GetSelectedOptions();
				not selections.empty()) {
			return static_cast<m2g::pb::SpriteType>(I(selections[0]));
		}
		return std::nullopt;
	}

	const UiPanelBlueprint gRightHud = {
		.name = "RightHud",
		.w = 19,
		.h = 72,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 2, .y = 6, .w = 15, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Set Rect", .onAction = [](MAYBE const widget::Text& self) -> UiAction {
						const auto selectedSprite = SelectedSprite();
						if (const auto* selection = M2_LEVEL.PrimarySelection(); selection->IsComplete()) {
							std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).SetRect(*selectedSprite, *selection->IntegerSelectionRectM());
							std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).LookUpAndStoreSpriteRect(*selectedSprite);
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 2, .y = 11, .w = 15, .h = 4,
				.variant = widget::TextBlueprint{
					.text = "Reset", .onAction = [](MAYBE const widget::Text& self) -> UiAction {
						if (const auto selectedSprite = SelectedSprite(); selectedSprite) {
							std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).Reset(*selectedSprite);
							std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).LookUpAndStoreSpriteRect(*selectedSprite);
						}
						return MakeContinueAction();
					}
				}
			}
		}
	};

	UiPanelBlueprint GenerateLeftHud(const pb::SpriteSheet& sheet) {
		return UiPanelBlueprint{
			.name = "LeftHud",
			.w = 19, .h = 72,
			.background_color = {0, 0, 0, 255},
			.widgets = {
				UiWidgetBlueprint{
					.name = "SpriteTypeSelection",
					.x = 0, .y = 0, .w = 19, .h = 72,
					.variant = widget::TextSelectionBlueprint{
						.line_count = 18,
						.onCreate = [=](MAYBE widget::TextSelection& self) {
							widget::TextSelectionBlueprint::Options options;
							for (const auto& sprite : sheet.sprites()) {
								if (sprite.type()) {
									options.emplace_back(pb::enum_name<>(sprite.type()), I(sprite.type()));
								}
							}
							self.SetOptions(std::move(options));
						},
						.onAction = [](const widget::TextSelection& self) -> UiAction {
							if (const auto selections = self.GetSelectedOptions(); not selections.empty()) {
								const auto selectedSpriteType = static_cast<m2g::pb::SpriteType>(I(selections[0]));
								std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).LookUpAndStoreSpriteRect(selectedSpriteType);
								const auto sprite_name = pb::enum_name(selectedSpriteType);
								M2_LEVEL.ShowMessage(sprite_name);
							} else {
								std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).LookUpAndStoreSpriteRect(std::nullopt);
							}
							return MakeContinueAction();
						}
					}
				}
			}
		};
	}
}

const UiPanelBlueprint m2::bulk_sheet_editor::gMainMenu = {
	.name = "MainMenu",
    .w = 16,
    .h = 9,
    .border_width = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        UiWidgetBlueprint{
        	.name = "ResourceSelection",
            .x = 1, .y = 1, .w = 14, .h = 5,
        	.variant = widget::TextSelectionBlueprint{
        		.line_count = 5,
        		.onCreate = [](widget::TextSelection& self) {
        			const auto& sheets = std::get<State>(M2_LEVEL.stateVariant).ReadSpriteSheetsFromFile();
        			widget::TextSelectionBlueprint::Options resources;
        			std::ranges::for_each(sheets.sheets(), [&resources](const auto& sheet) {
						if (!sheet.resource().empty()) {
							resources.emplace_back(widget::TextSelectionBlueprint::Option{.text = sheet.resource(), .return_value = sheet.resource()});
						}
					});
        			std::ranges::sort(resources, widget::TextSelectionBlueprint::OptionsSorter);
        			self.SetOptions(std::move(resources));
        		}
        	}
        },
        UiWidgetBlueprint{
            .x = 1, .y = 7, .w = 14, .h = 1,
            .variant = widget::TextBlueprint{
                .text = "SELECT",
                .keyboardShortcut = m2g::pb::RETURN,
                .onAction = [](MAYBE const widget::Text& self) -> UiAction {
	                if (const auto selections = self.Parent().FindWidget<widget::TextSelection>("ResourceSelection")->GetSelectedOptions();
	                		not selections.empty()) {
		                if (const auto spriteSheet = std::get<State>(M2_LEVEL.stateVariant).SelectResource(std::get<std::string>(selections[0]))) {
		                	M2_LEVEL.ReplaceLeftHud(std::make_unique<UiPanelBlueprint>(GenerateLeftHud(*spriteSheet)), M2_GAME.Dimensions().LeftHud());
	                		M2_LEVEL.ReplaceRightHud(&gRightHud, M2_GAME.Dimensions().RightHud());
	                		return MakeReturnAction();
	                	}
                	}
                	return MakeContinueAction();
                }
            }
        }
    }
};
