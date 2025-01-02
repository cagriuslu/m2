#include <m2/Game.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/ui/widget/TextSelection.h>

using namespace m2;
using namespace m2::ui;

const widget::TextBlueprint right_hud_set_rect_button = {
    .text = "Set Rect", .on_action = [](MAYBE const widget::Text& self) -> Action {
	    std::get<bsedit::State>(M2_LEVEL.type_state).set_rect();
	    return MakeContinueAction();
    }};
const widget::TextBlueprint right_hud_reset_button = {
    .text = "Reset", .on_action = [](MAYBE const widget::Text& self) -> Action {
	    std::get<bsedit::State>(M2_LEVEL.type_state).reset();
	    return MakeContinueAction();
    }};
const PanelBlueprint ui::bulk_sheet_editor_right_hud = {
    .w = 19,
    .h = 72,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        WidgetBlueprint{
            .x = 2,
            .y = 2,
            .w = 15,
            .h = 3,
            .variant = widget::TextSelectionBlueprint{
                .on_create = [](MAYBE widget::TextSelection& self) {
	                if (auto selected_ss = std::get<bsedit::State>(M2_LEVEL.type_state).selected_sprite_sheet()) {
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
                .on_action = [](widget::TextSelection& self) -> Action {
	                if (auto selected_sprite_type = static_cast<m2g::pb::SpriteType>(std::get<int>(self.selections()[0]))) {
		                std::get<bsedit::State>(M2_LEVEL.type_state).select_sprite(selected_sprite_type);
		                return MakeContinueAction();
	                }
	                throw M2_ERROR("Implementation error: Unknown sprite type ended up in sprite selection list");
                }
			}
		},
        WidgetBlueprint{.x = 2, .y = 6, .w = 15, .h = 4, .variant = right_hud_set_rect_button},
        WidgetBlueprint{.x = 2, .y = 11, .w = 15, .h = 4, .variant = right_hud_reset_button}}};

const PanelBlueprint ui::bulk_sheet_editor_left_hud = {
    .w = 19, .h = 72, .background_color = {0, 0, 0, 255}, .widgets = {}};

const widget::TextSelectionBlueprint resource_selection = {
    .on_create = [](MAYBE widget::TextSelection& self) {
	    const auto& pb_sheets = std::get<bsedit::State>(M2_LEVEL.type_state).sprite_sheets();
	    // Gather the list of resources
		widget::TextSelectionBlueprint::Options resources;
	    std::for_each(pb_sheets.sheets().cbegin(), pb_sheets.sheets().cend(), [&resources](const auto& sheet) {
		    if (!sheet.resource().empty()) {
			    resources.emplace_back(widget::TextSelectionBlueprint::Option{sheet.resource(), sheet.resource()});
		    }
	    });
	    // Sort the list
	    std::sort(resources.begin(), resources.end(), widget::TextSelectionBlueprint::OptionsSorter);
		self.set_options(std::move(resources));
    },
    .on_action = [](const widget::TextSelection& self) -> ui::Action {
	    std::get<bsedit::State>(M2_LEVEL.type_state).select_resource(std::get<std::string>(self.selections()[0]));
	    return MakeContinueAction();
    }};
const PanelBlueprint ui::bulk_sheet_editor_main_menu = {
    .w = 160,
    .h = 90,
    .border_width = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        WidgetBlueprint{
            .x = 35,
            .y = 55,
            .w = 90,
            .h = 10,
            .variant = resource_selection},
        WidgetBlueprint{
            .x = 35,
            .y = 70,
            .w = 40,
            .h = 10,
            .variant =
                widget::TextBlueprint{
                    .text = "QUIT",
                    .kb_shortcut = SDL_SCANCODE_Q,
                    .on_action = [](MAYBE const widget::Text& self) -> Action { return MakeQuitAction(); },
                }},
        WidgetBlueprint{
            .x = 85,
            .y = 70,
            .w = 40,
            .h = 10,
            .variant = widget::TextBlueprint{
                .text = "SELECT",
                .kb_shortcut = SDL_SCANCODE_RETURN,
                .on_action = [](MAYBE const widget::Text& self) -> Action {
	                if (std::get<bsedit::State>(M2_LEVEL.type_state).select()) {
		                M2_LEVEL.right_hud_ui_panel.emplace(&bulk_sheet_editor_right_hud, M2_GAME.Dimensions().RightHud());
		                // TODO return selection instead
		                return MakeReturnAction();
	                } else {
		                return MakeContinueAction();
	                }
                }}}}};

const PanelBlueprint ui::bulk_sheet_editor_pause_menu = {
    .w = 160,
    .h = 90,
    .border_width = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {WidgetBlueprint{
        .x = 60,
        .y = 40,
        .w = 40,
        .h = 10,
        .variant = widget::TextBlueprint{
            .text = "QUIT",
            .kb_shortcut = SDL_SCANCODE_Q,
            .on_action = [](MAYBE const widget::Text& self) -> Action { return MakeQuitAction(); },
        }}}};
