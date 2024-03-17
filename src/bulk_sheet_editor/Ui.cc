#include <m2/Game.h>
#include <m2/bulk_sheet_editor/Ui.h>
#include <m2/ui/widget/TextSelection.h>

using namespace m2;
using namespace m2::ui;

const widget::TextBlueprint right_hud_set_rect_button = {
    .initial_text = "Set Rect", .on_action = [](MAYBE const widget::Text& self) -> Action {
	    std::get<bsedit::State>(LEVEL.type_state).set_rect();
	    return make_continue_action();
    }};
const widget::TextBlueprint right_hud_reset_button = {
    .initial_text = "Reset", .on_action = [](MAYBE const widget::Text& self) -> Action {
	    std::get<bsedit::State>(LEVEL.type_state).reset();
	    return make_continue_action();
    }};
const Blueprint ui::bulk_sheet_editor_right_hud = {
    .w = 19,
    .h = 72,
    .border_width_px = 1,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        WidgetBlueprint{
            .x = 2,
            .y = 2,
            .w = 15,
            .h = 3,
            .variant = widget::TextSelectionBlueprint{
                .on_create =
                    [](MAYBE const widget::TextSelection& self) -> std::optional<widget::TextSelectionBlueprint::Options> {
	                if (auto selected_ss = std::get<bsedit::State>(LEVEL.type_state).selected_sprite_sheet();
	                    selected_ss) {
		                widget::TextSelectionBlueprint::Options options;
		                for (const auto& sprite : selected_ss->sprites()) {
			                options.emplace_back(pb::enum_name<>(sprite.type()));
		                }
		                return options;
	                }
	                return std::nullopt;
                },
                .on_action = [](const widget::TextSelection& self) -> Action {
	                m2g::pb::SpriteType selected_sprite_type;
	                if (SpriteType_Parse(self.selection(), &selected_sprite_type)) {
		                std::get<bsedit::State>(LEVEL.type_state).select_sprite(selected_sprite_type);
		                return make_continue_action();
	                }
	                throw M2FATAL("Implementation error: Unknown sprite type ended up in sprite selection list");
                }}},
        WidgetBlueprint{.x = 2, .y = 6, .w = 15, .h = 4, .border_width_px = 1, .variant = right_hud_set_rect_button},
        WidgetBlueprint{.x = 2, .y = 11, .w = 15, .h = 4, .border_width_px = 1, .variant = right_hud_reset_button}}};

const Blueprint ui::bulk_sheet_editor_left_hud = {
    .w = 19, .h = 72, .border_width_px = 1, .background_color = {0, 0, 0, 255}, .widgets = {}};

const widget::TextSelectionBlueprint resource_selection = {
    .on_create = [](MAYBE const widget::TextSelection& self) -> std::optional<widget::TextSelectionBlueprint::Options> {
	    const auto& pb_sheets = std::get<bsedit::State>(LEVEL.type_state).sprite_sheets();
	    // Gather the list of resources
	    std::vector<std::string> resources;
	    std::for_each(pb_sheets.sheets().cbegin(), pb_sheets.sheets().cend(), [&resources](const auto& sheet) {
		    if (!sheet.resource().empty()) {
			    resources.emplace_back(sheet.resource());
		    }
	    });
	    // Sort the list
	    std::sort(resources.begin(), resources.end());
	    return resources;
    },
    .on_action = [](const widget::TextSelection& self) -> ui::Action {
	    std::get<bsedit::State>(LEVEL.type_state).select_resource(self.selection());
	    return make_continue_action();
    }};
const Blueprint ui::bulk_sheet_editor_main_menu = {
    .w = 160,
    .h = 90,
    .border_width_px = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {
        WidgetBlueprint{
            .x = 35,
            .y = 55,
            .w = 90,
            .h = 10,
            .border_width_px = 1,
            .padding_width_px = 2,
            .variant = resource_selection},
        WidgetBlueprint{
            .x = 35,
            .y = 70,
            .w = 40,
            .h = 10,
            .border_width_px = 1,
            .padding_width_px = 2,
            .variant =
                widget::TextBlueprint{
                    .initial_text = "QUIT",
                    .kb_shortcut = SDL_SCANCODE_Q,
                    .on_action = [](MAYBE const widget::Text& self) -> Action { return make_quit_action(); },
                }},
        WidgetBlueprint{
            .x = 85,
            .y = 70,
            .w = 40,
            .h = 10,
            .border_width_px = 1,
            .padding_width_px = 2,
            .variant = widget::TextBlueprint{
                .initial_text = "SELECT",
                .kb_shortcut = SDL_SCANCODE_RETURN,
                .on_action = [](MAYBE const widget::Text& self) -> Action {
	                if (std::get<bsedit::State>(LEVEL.type_state).select()) {
		                LEVEL.right_hud_ui_state.emplace(&bulk_sheet_editor_right_hud);
		                LEVEL.right_hud_ui_state->update_positions(GAME.dimensions().right_hud);
		                // TODO return selection instead
		                return make_return_action();
	                } else {
		                return make_continue_action();
	                }
                }}}}};

const Blueprint ui::bulk_sheet_editor_pause_menu = {
    .w = 160,
    .h = 90,
    .border_width_px = 0,
    .background_color = {0, 0, 0, 255},
    .widgets = {WidgetBlueprint{
        .x = 60,
        .y = 40,
        .w = 40,
        .h = 10,
        .border_width_px = 1,
        .padding_width_px = 2,
        .variant = widget::TextBlueprint{
            .initial_text = "QUIT",
            .kb_shortcut = SDL_SCANCODE_Q,
            .on_action = [](MAYBE const widget::Text& self) -> Action { return make_quit_action(); },
        }}}};
