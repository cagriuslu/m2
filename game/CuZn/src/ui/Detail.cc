#include <cuzn/ui/Detail.h>
#include <m2/Level.h>
#include <m2/ui/Action.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextListSelection.h>
#include <cuzn/journeys/BuildJourney.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

Blueprint generate_cards_window(m2g::pb::ItemType exclude_card) {
	return Blueprint{
		.w = 60,
		.h = 40,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.name = "CardSelection",
				.x = 2,
				.y = 2,
				.w = 56,
				.h = 30,
				.variant = TextListSelectionBlueprint{
					.line_count = 8,
					.allow_multiple_selection = false,
					.show_scroll_bar = false,
					.on_create = [&exclude_card](MAYBE const TextListSelection& self) -> std::optional<TextListSelectionBlueprint::Options> {
						auto cards = m2::generate_named_item_types_transformer(
							{m2g::pb::ITEM_CATEGORY_CITY_CARD, m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD, m2g::pb::ITEM_CATEGORY_WILD_CARD})(M2_PLAYER.character())
							| std::views::filter([exclude_card](auto item_type) { return exclude_card != item_type; });
						TextListSelectionBlueprint::Options options;
						std::ranges::for_each(cards, [&options](auto item_type) {
							options.emplace_back(M2_GAME.get_named_item(item_type).in_game_name());
						});
						return options;
					}
				}
			},
			WidgetBlueprint{
				.x = 2,
				.y = 34,
				.w = 56,
				.h = 4,
				.variant = TextBlueprint{
					.text = "OK",
					.on_action = [](const Text& self) -> Action {
						// Find the other blueprint
						auto* card_selection = self.parent().find_first_widget_by_name<TextListSelection>("CardSelection");
						if (card_selection && not card_selection->selection().empty()) {
							m2g::pb::ItemType item_type;
							if (m2g::pb::ItemType_Parse(card_selection->selection()[0], &item_type)) {
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

m2::RectF cards_window_ratio() {
	return m2::RectF{0.25f, 0.15f, 0.5f, 0.7f};
}

std::optional<m2g::pb::ItemType> ask_for_card_selection(m2g::pb::ItemType exclude_card) {
	LOG_INFO("Asking player to select a card...");
	std::optional<m2g::pb::ItemType> selected_card;
	State::create_execute_sync(std::make_unique<Blueprint>(generate_cards_window(exclude_card)), M2_GAME.dimensions().game_and_hud.ratio(cards_window_ratio()))
		.if_void_return([&]() {
			LOG_INFO("Card selection cancelled");
		})
		.if_return<m2g::pb::ItemType>([&selected_card](auto picked_card) {
			LOG_INFO("Card selected", m2g::pb::ItemType_Name(picked_card));
			selected_card = picked_card;
		});
	return selected_card;
}

std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2) {
	LOG_INFO("Asking player to select an industry...");

	auto blueprint = Blueprint{
		.w = 60, .h = 40,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 57, .y = 0, .w = 3, .h = 3,
				.variant = TextBlueprint{
					.text = "X",
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action();
					}
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 14, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.text = M2_GAME.get_named_item(industry_1).in_game_name(),
					.on_action = [industry_1](MAYBE const Text& self) -> Action {
						return make_return_action<m2g::pb::ItemType>(industry_1);
					}
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 21, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.text = M2_GAME.get_named_item(industry_2).in_game_name(),
					.on_action = [industry_2](MAYBE const Text& self) -> Action {
						return make_return_action<m2g::pb::ItemType>(industry_2);
					}
				}
			}
		}
	};

	std::optional<m2g::pb::ItemType> selected_industry;
	m2::ui::State::create_execute_sync(
		std::make_unique<m2::ui::Blueprint>(blueprint),
		M2_GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}))
		.if_void_return([&]() {
			LOG_INFO("Industry selection cancelled");
		})
		.if_return<m2g::pb::ItemType>([&](auto industry) {
			LOG_INFO("Industry selected", m2g::pb::ItemType_Name(industry));
			selected_industry = industry;
		});
	return selected_industry;
}

bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text) {
	auto blueprint = Blueprint{
		.w = 60, .h = 40,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 5, .y = 5, .w = 50, .h = 5,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.text = question1,
					.font_size = 4.5f,
					.alignment = TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 10, .w = 50, .h = 5,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.text = question2,
					.font_size = 4.5f,
					.alignment = TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 30, .w = 15, .h = 5,
				.variant = TextBlueprint{
					.text = decline_text,
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action<bool>(false);
					}
				}
			},
			WidgetBlueprint{
				.x = 25, .y = 30, .w = 30, .h = 5,
				.variant = TextBlueprint{
					.text = accept_text,
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action<bool>(true);
					}
				}
			}
		}
	};

	bool selection;
	State::create_execute_sync(&blueprint, M2_GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}))
		.if_return<bool>([&](auto result) { selection = result; });
	return selection;
}
