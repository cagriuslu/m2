#include <cuzn/ui/Cards.h>
#include <cuzn/Detail.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Text.h>

using namespace m2;
using namespace m2::widget;

namespace {
	RGB cards_window_card_color(Card card) {
		switch (M2_GAME.named_items[card].Category()) {
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

	TextSelectionBlueprint::Options list_cards_as_selection_options(const m2g::pb::ItemType exclude_card_1, const m2g::pb::ItemType exclude_card_2) {
		const auto card_filter = GenerateNamedItemTypesFilter(
				{m2g::pb::ITEM_CATEGORY_CITY_CARD,
				m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD,
				m2g::pb::ITEM_CATEGORY_WILD_CARD});
		const auto cards = card_filter(M2_PLAYER.GetCharacter());

		Card filter_card_1 = exclude_card_1, filter_card_2 = exclude_card_2;
		TextSelectionBlueprint::Options options;
		std::ranges::for_each(cards, [&filter_card_1, &filter_card_2, &options](const auto card) {
				if (filter_card_1 && card == filter_card_1) {
					filter_card_1 = static_cast<Card>(0);
				} else if (filter_card_2 && card == filter_card_2) {
					filter_card_2 = static_cast<Card>(0);
				} else {
					// Add option
					options.emplace_back(TextSelectionBlueprint::Option{M2_GAME.GetNamedItem(card).in_game_name(), static_cast<int>(card),cards_window_card_color(card)});
				}});

		// Sort first by type, then by alphabetically
		std::ranges::sort(options,
				[](const TextSelectionBlueprint::Option& a, const TextSelectionBlueprint::Option& b) {
					const auto a_category = M2_GAME.named_items[static_cast<Card>(std::get<int>(a.return_value))].Category();
					const auto b_category = M2_GAME.named_items[static_cast<Card>(std::get<int>(b.return_value))].Category();
					if (a_category != b_category) {
						return a_category < b_category;
					}
					return a.text < b.text;
				});

		return options;
	}
}

m2::RectF cards_window_ratio() {
	return m2::RectF{0.30f, 0.10f, 0.4f, 0.8f};
}
m2::RectF cards_panel_ratio() {
	return m2::RectF{
		M2_GAME.Dimensions().HudWidthToGameAndHudWidthRatio() + M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio() - 0.16f,
		0.6f,
		0.16f,
		0.4f};
}

UiPanelBlueprint generate_cards_window(const std::string& msg, m2g::pb::ItemType exclude_card_1, m2g::pb::ItemType exclude_card_2, bool blocking_window) {
	auto panel_blueprint = UiPanelBlueprint{
		.name = "CardsPanel",
		.w = 24,
		.h = 24,
		.border_width = 0.001f,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1,
				.y = 1,
				.w = 22,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{ .text = msg }
			},
			UiWidgetBlueprint{
				.name = "CardSelection",
				.x = 1,
				.y = 4,
				.w = 22,
				.h = 16,
				.variant = TextSelectionBlueprint{
					.line_count = 8,
					.allow_multiple_selection = false,
					.show_scroll_bar = false,
					.onCreate = [=](MAYBE TextSelection& self) {
						self.set_options(list_cards_as_selection_options(exclude_card_1, exclude_card_2));
					}
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 21,
				.w = blocking_window ? 10 : 22,
				.h = 2,
				.variant = TextBlueprint{
					.text = blocking_window ? "OK" : "Close",
					.onAction = [](const Text& self) -> UiAction {
						// Find the other blueprint
						if (auto* card_selection = self.Parent().FindWidget<TextSelection>("CardSelection")) {
							if (auto selections = card_selection->selections(); not selections.empty()) {
								auto item_type = static_cast<m2g::pb::ItemType>(std::get<int>(selections[0]));
								return MakeReturnAction<m2g::pb::ItemType>(item_type);
							}
						}
						return MakeReturnAction();
					}
				}
			}
		}
	};

	if (blocking_window) {
		panel_blueprint.widgets.emplace_back(
			UiWidgetBlueprint{
				.x = 13,
				.y = 21,
				.w = 10,
				.h = 2,
				.variant = TextBlueprint{
					.text = "Cancel",
					.onAction = [](MAYBE const Text& self) -> UiAction {
						// Return empty return
						return MakeReturnAction();
					}
				}
			}
		);
	}

	return panel_blueprint;
}

std::optional<m2g::pb::ItemType> ask_for_card_selection(m2g::pb::ItemType exclude_card_1, m2g::pb::ItemType exclude_card_2) {
	LOG_INFO("Asking player to select a card");
	std::optional<m2g::pb::ItemType> selected_card;
	auto background = M2_GAME.DrawGameToTexture(M2_LEVEL.Camera()->position);
	UiPanel::create_and_run_blocking(std::make_unique<UiPanelBlueprint>(
			generate_cards_window("Select card to discard", exclude_card_1, exclude_card_2, true)), cards_window_ratio(), std::move(background))
			.IfQuit([] { M2_GAME.quit = true; })
			.IfVoidReturn([&]() {
				LOG_INFO("Card selection cancelled");
			})
			.IfReturn<m2g::pb::ItemType>([&selected_card](const auto& picked_card) {
				LOG_INFO("Card selected", m2g::pb::ItemType_Name(picked_card));
				selected_card = picked_card;
			});
	return selected_card;
}
