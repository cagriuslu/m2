#include <cuzn/Ui.h>
#include <m2/Level.h>
#include <m2/ui/Action.h>
#include <m2/Game.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextListSelection.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

Blueprint cuzn::generate_cards_window(bool return_selection) {
	return Blueprint{
		.w = 60,
		.h = 40,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 57,
				.y = 0,
				.w = 3,
				.h = 3,
				.variant =
				TextBlueprint{
					.initial_text = "X",
					.on_action = [return_selection](MAYBE const Text& self) -> Action {
						if (not return_selection) {
							// Remove self from custom UI dialog
							LEVEL.remove_custom_ui_dialog();
						}
						return make_return_action<m2::Void>();
					}
				}
			},
			WidgetBlueprint{
				.name = "CardSelection",
				.x = 5,
				.y = 5,
				.w = 30,
				.h = 30,
				.variant = TextListSelectionBlueprint{
					.line_count = 8,
					.allow_multiple_selection = false,
					.show_scroll_bar = false,
					.on_create =
					[](MAYBE const TextListSelection& self) -> std::optional<TextListSelectionBlueprint::Options> {
						TextListSelectionBlueprint::Options options;
						// Iterate over the cards of the player
						for (auto item_it = LEVEL.player()->character().find_items(m2g::pb::ITEM_CATEGORY_CARD);
							item_it != LEVEL.player()->character().end_items(); ++item_it) {
							options.emplace_back(m2g::pb::ItemType_Name(item_it->type()));
						}
						return options;
					}
				}
			},
			// TODO Card details
			WidgetBlueprint{
				.initially_enabled = return_selection,
				.x = 40,
				.y = 30,
				.w = 15,
				.h = 5,
				.variant = TextBlueprint{
					.initial_text = "Select",
					.on_action = [](const Text& self) -> Action {
						// Find the other blueprint
						auto* card_selection = self.parent().find_first_widget_by_name<TextListSelection>("CardSelection");
						if (card_selection && not card_selection->selection().empty()) {
							m2g::pb::ItemType item_type;
							if (m2g::pb::ItemType_Parse(card_selection->selection()[0], &item_type)) {
								return make_return_action<m2g::pb::ItemType>(item_type);
							}
						}
						// Else, don't return
						return make_continue_action();
					}
				}
			}
		}
	};
}
