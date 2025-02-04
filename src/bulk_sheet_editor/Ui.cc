#include <m2/Game.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextSelection.h>

using namespace m2;
using namespace m2;

const widget::TextBlueprint right_hud_set_rect_button = {
    .text = "Set Rect", .onAction = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).set_rect();
	    return MakeContinueAction();
    }
};
const widget::TextBlueprint right_hud_reset_button = {
    .text = "Reset", .onAction = [](MAYBE const widget::Text& self) -> UiAction {
	    std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).reset();
	    return MakeContinueAction();
    }
};
const UiPanelBlueprint m2::bulk_sheet_editor::gRightHud = {
	.name = "RightHud",
    .w = 19,
    .h = 72,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        UiWidgetBlueprint{
            .x = 2,
            .y = 2,
            .w = 15,
            .h = 3,
            .variant = widget::TextSelectionBlueprint{
                .onCreate = [](MAYBE widget::TextSelection& self) {
	                if (auto selected_ss = std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).ReadSelectedSpriteSheetFromFile()) {
		                widget::TextSelectionBlueprint::Options options;
		                for (const auto& sprite : selected_ss->sprites()) {
			                options.emplace_back(
								widget::TextSelectionBlueprint::Option{
									pb::enum_name<>(sprite.type()),
									widget::TextSelectionBlueprint::ReturnValue{I(sprite.type())}
								}
							);
		                }
						self.set_options(std::move(options));
	                }
                },
                .onAction = [](widget::TextSelection& self) -> UiAction {
	                if (auto selected_sprite_type = static_cast<m2g::pb::SpriteType>(std::get<int>(self.selections()[0]))) {
		                std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).SelectSpriteType(selected_sprite_type);
		                return MakeContinueAction();
	                }
                	return MakeContinueAction();
	                // throw M2_ERROR("Implementation error: Unknown sprite type ended up in sprite selection list");
                }
			}
		},
        UiWidgetBlueprint{.x = 2, .y = 6, .w = 15, .h = 4, .variant = right_hud_set_rect_button},
        UiWidgetBlueprint{.x = 2, .y = 11, .w = 15, .h = 4, .variant = right_hud_reset_button}
    }
};

const UiPanelBlueprint m2::bulk_sheet_editor::gLeftHud = {
	.name = "LeftHud",
    .w = 19, .h = 72, .background_color = {0, 0, 0, 255}, .widgets = {}
};

const UiPanelBlueprint m2::bulk_sheet_editor::gMainMenu = {
	.name = "MainMenu",
    .w = 16,
    .h = 9,
    .border_width = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        UiWidgetBlueprint{
        	.name = "ResourceSelection",
            .x = 1,
            .y = 1,
            .w = 14,
            .h = 5,
        	.variant = widget::TextSelectionBlueprint{
        		.line_count = 5,
        		.onCreate = [](widget::TextSelection& self) {
        			const auto& sheets = std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).ReadSpriteSheetsFromFile();
        			widget::TextSelectionBlueprint::Options resources;
        			std::ranges::for_each(sheets.sheets(), [&resources](const auto& sheet) {
						if (!sheet.resource().empty()) {
							resources.emplace_back(widget::TextSelectionBlueprint::Option{.text = sheet.resource(), .return_value = sheet.resource()});
						}
					});
        			std::ranges::sort(resources, widget::TextSelectionBlueprint::OptionsSorter);
        			self.set_options(std::move(resources));
        		}
        	}
        },
        UiWidgetBlueprint{
            .x = 1,
            .y = 7,
            .w = 14,
            .h = 1,
            .variant = widget::TextBlueprint{
                .text = "SELECT",
                .kb_shortcut = SDL_SCANCODE_RETURN,
                .onAction = [](MAYBE const widget::Text& self) -> UiAction {
	                if (const auto selections = self.Parent().find_first_widget_by_name<widget::TextSelection>("ResourceSelection")->selections();
	                		not selections.empty()) {
		                if (const auto& selectedResource = std::get<std::string>(selections[0]);
		                		std::get<bulk_sheet_editor::State>(M2_LEVEL.stateVariant).SelectSpriteSheetResource(selectedResource)) {
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
