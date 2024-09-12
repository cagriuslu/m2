#include <cuzn/ui/Cards.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Text.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

m2::RectF cards_window_ratio() {
	return m2::RectF{0.30f, 0.10f, 0.4f, 0.8f};
}

PanelBlueprint generate_cards_window(const std::string& msg, m2g::pb::ItemType exclude_card_1, m2g::pb::ItemType exclude_card_2) {
	return PanelBlueprint{
		.w = 24,
		.h = 24,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 1,
				.y = 1,
				.w = 22,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{ .text = msg }
			},
			WidgetBlueprint{
				.name = "CardSelection",
				.x = 1,
				.y = 4,
				.w = 22,
				.h = 16,
				.variant = TextSelectionBlueprint{
					.line_count = 8,
					.allow_multiple_selection = false,
					.show_scroll_bar = false,
					.on_create = [=](MAYBE TextSelection& self) {
						auto cards = m2::generate_named_item_types_transformer(
							{m2g::pb::ITEM_CATEGORY_CITY_CARD, m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD, m2g::pb::ITEM_CATEGORY_WILD_CARD})(M2_PLAYER.character());
						Card filter_card_1 = exclude_card_1, filter_card_2 = exclude_card_2;
						TextSelectionBlueprint::Options options;
						std::ranges::for_each(cards, [&filter_card_1, &filter_card_2, &options](auto item_type) {
							if (filter_card_1 && item_type == filter_card_1) {
								filter_card_1 = static_cast<Card>(0);
							} else if (filter_card_2 && item_type == filter_card_2) {
								filter_card_2 = static_cast<Card>(0);
							} else {
								options.emplace_back(M2_GAME.get_named_item(item_type).in_game_name(), static_cast<int>(item_type));
							}
						});
						self.set_options(options);
					}
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 21,
				.w = 22,
				.h = 2,
				.variant = TextBlueprint{
					.text = "OK",
					.on_action = [](const Text& self) -> Action {
						// Find the other blueprint
						if (auto* card_selection = self.parent().find_first_widget_by_name<TextSelection>("CardSelection")) {
							if (auto selections = card_selection->selections(); not selections.empty()) {
								auto item_type = static_cast<m2g::pb::ItemType>(std::get<int>(selections[0]));
								return make_return_action<m2g::pb::ItemType>(item_type);
							}
						}
						return make_return_action();
					}
				}
			}
		}
	};
}

std::optional<m2g::pb::ItemType> ask_for_card_selection(m2g::pb::ItemType exclude_card_1, m2g::pb::ItemType exclude_card_2) {
	LOG_INFO("Asking player to select a card...");
	std::optional<m2g::pb::ItemType> selected_card;
	Panel::create_execute_sync(std::make_unique<PanelBlueprint>(generate_cards_window("Select card to discard", exclude_card_1, exclude_card_2)), M2_GAME.dimensions().game_and_hud.ratio(cards_window_ratio()))
		.if_void_return([&]() {
			LOG_INFO("Card selection cancelled");
		})
		.if_return<m2g::pb::ItemType>([&selected_card](auto picked_card) {
			LOG_INFO("Card selected", m2g::pb::ItemType_Name(picked_card));
			selected_card = picked_card;
		});
	return selected_card;
}
