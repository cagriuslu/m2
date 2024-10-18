#include <cuzn/ui/Cards.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Text.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

namespace {
	RGB cards_window_card_color(Card card) {
		switch (M2_GAME.named_items[card].category()) {
			case m2g::pb::ITEM_CATEGORY_WILD_CARD:
				return {255, 200, 71};
			case m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD:
				return {255, 64, 64};
			case m2g::pb::ITEM_CATEGORY_CITY_CARD:
				return {123, 123, 255};
			default:
				return {255, 255, 255};
		}
	}
}

m2::RectF cards_window_ratio() {
	return m2::RectF{0.30f, 0.10f, 0.4f, 0.8f};
}

PanelBlueprint generate_cards_window(const std::string& msg, m2g::pb::ItemType exclude_card_1, m2g::pb::ItemType exclude_card_2, bool cancel_button) {
	auto panel_blueprint = PanelBlueprint{
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
						auto card_filter = m2::generate_named_item_types_filter(
							{m2g::pb::ITEM_CATEGORY_CITY_CARD,
							 m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD,
							 m2g::pb::ITEM_CATEGORY_WILD_CARD});
						auto cards = card_filter(M2_PLAYER.character());

						Card filter_card_1 = exclude_card_1, filter_card_2 = exclude_card_2;
						TextSelectionBlueprint::Options options;
						std::ranges::for_each(cards, [&filter_card_1, &filter_card_2, &options](auto card) {
							if (filter_card_1 && card == filter_card_1) {
								filter_card_1 = static_cast<Card>(0);
							} else if (filter_card_2 && card == filter_card_2) {
								filter_card_2 = static_cast<Card>(0);
							} else {
								// Add options
								options.emplace_back(
									widget::TextSelectionBlueprint::Option{
										M2_GAME.get_named_item(card).in_game_name(), static_cast<int>(card),
										cards_window_card_color(card)});
								// Sort first by type, then by alphabetically
								std::sort(options.begin(), options.end(),
									[](const widget::TextSelectionBlueprint::Option& a, const widget::TextSelectionBlueprint::Option& b) {
									auto a_category = M2_GAME.named_items[static_cast<Card>(std::get<int>(a.return_value))].category();
									auto b_category = M2_GAME.named_items[static_cast<Card>(std::get<int>(b.return_value))].category();
									if (a_category != b_category) {
										return a_category < b_category;
									} else {
										return a.text < b.text;
									}
								});
							}
						});
						self.set_options(options);
					}
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 21,
				.w = cancel_button ? 10 : 22,
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

	if (cancel_button) {
		panel_blueprint.widgets.emplace_back(
			WidgetBlueprint{
				.x = 13,
				.y = 21,
				.w = 10,
				.h = 2,
				.variant = TextBlueprint{
					.text = "Cancel",
					.on_action = [](MAYBE const Text& self) -> Action {
						// Return empty return
						return make_return_action();
					}
				}
			}
		);
	}

	return panel_blueprint;
}

std::optional<m2g::pb::ItemType> ask_for_card_selection(m2g::pb::ItemType exclude_card_1, m2g::pb::ItemType exclude_card_2) {
	LOG_INFO("Asking player to select a card...");
	std::optional<m2g::pb::ItemType> selected_card;
	auto background = M2_GAME.draw_game_to_texture(M2_LEVEL.camera()->position);
	Panel::create_and_run_blocking(std::make_unique<PanelBlueprint>(
		generate_cards_window("Select card to discard", exclude_card_1, exclude_card_2, true)),
			cards_window_ratio(), std::move(background))
		.if_void_return([&]() {
			LOG_INFO("Card selection cancelled");
		})
		.if_return<m2g::pb::ItemType>([&selected_card](auto picked_card) {
			LOG_INFO("Card selected", m2g::pb::ItemType_Name(picked_card));
			selected_card = picked_card;
		});
	return selected_card;
}
