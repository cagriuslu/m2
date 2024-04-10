#include <cuzn/Ui.h>
#include <m2/Level.h>
#include <m2/ui/Action.h>
#include <m2/Game.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextListSelection.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

Blueprint cuzn::generate_cards_window(bool has_return_button) {
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
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action();
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
						for (auto item_it = LEVEL.player()->character().find_items(m2g::pb::ITEM_CATEGORY_WILD_CARD);
							item_it != LEVEL.player()->character().end_items(); ++item_it) {
							options.emplace_back(m2g::pb::ItemType_Name(item_it->type()));
						}
						for (auto item_it = LEVEL.player()->character().find_items(m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD);
							item_it != LEVEL.player()->character().end_items(); ++item_it) {
							options.emplace_back(m2g::pb::ItemType_Name(item_it->type()));
						}
						for (auto item_it = LEVEL.player()->character().find_items(m2g::pb::ITEM_CATEGORY_CITY_CARD);
							item_it != LEVEL.player()->character().end_items(); ++item_it) {
							options.emplace_back(m2g::pb::ItemType_Name(item_it->type()));
						}
						return options;
					}
				}
			},
			// TODO Card details
			WidgetBlueprint{
				.initially_enabled = has_return_button,
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

std::optional<m2g::pb::ItemType> cuzn::ask_for_card_selection() {
	LOG_INFO("Asking player to select a card...");
	std::optional<m2g::pb::ItemType> selected_card;
	m2::ui::State::create_execute_sync(std::make_unique<m2::ui::Blueprint>(generate_cards_window(true)), GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}))
		.if_void_return([&]() {
			LOG_INFO("Card selection cancelled");
		})
		.if_return<m2g::pb::ItemType>([&selected_card](auto picked_card) {
			LOG_INFO("Card selected", m2g::pb::ItemType_Name(picked_card));
			selected_card = picked_card;
		});
	return selected_card;
}

m2::ui::Blueprint cuzn::generate_industry_selection_window(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2) {
	return Blueprint{
		.w = 60, .h = 40,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 57, .y = 0, .w = 3, .h = 3,
				.variant = TextBlueprint{
					.initial_text = "X",
					.on_action = [](MAYBE const Text& self) -> Action {
						return make_return_action();
					}
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 14, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.initial_text = GAME.get_named_item(industry_1).in_game_name(),
					.on_action = [industry_1](MAYBE const Text& self) -> Action {
						return make_return_action<m2g::pb::ItemType>(industry_1);
					}
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 21, .w = 50, .h = 5,
				.variant = TextBlueprint{
					.initial_text = GAME.get_named_item(industry_2).in_game_name(),
					.on_action = [industry_2](MAYBE const Text& self) -> Action {
						return make_return_action<m2g::pb::ItemType>(industry_2);
					}
				}
			}
		}
	};
}

std::optional<m2g::pb::ItemType> cuzn::ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2) {
	LOG_INFO("Asking player to select an industry...");
	std::optional<m2g::pb::ItemType> selected_industry;
	m2::ui::State::create_execute_sync(
		std::make_unique<m2::ui::Blueprint>(generate_industry_selection_window(industry_1, industry_2)),
		GAME.dimensions().game_and_hud.ratio({0.15f, 0.15f, 0.7f, 0.7f}))
		.if_void_return([&]() {
			LOG_INFO("Industry selection cancelled");
		})
		.if_return<m2g::pb::ItemType>([&](auto industry) {
			LOG_INFO("Industry selected", m2g::pb::ItemType_Name(industry));
			selected_industry = industry;
		});
	return selected_industry;
}

m2::ui::Blueprint cuzn::generate_build_confirmation(m2g::pb::ItemType card, m2g::pb::ItemType city, m2g::pb::ItemType industry) {
	auto card_name = GAME.get_named_item(card).in_game_name();
	auto city_name = GAME.get_named_item(city).in_game_name();
	auto industry_name = GAME.get_named_item(industry).in_game_name();
	return Blueprint{
		.w = 60, .h = 40,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 5, .y = 5, .w = 50, .h = 5,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.initial_text = "Build " + industry_name + " in " + city_name,
					.font_size = 4.5f,
					.alignment = m2::ui::TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 10, .w = 50, .h = 5,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.initial_text = "using " + card_name + " card?",
					.font_size = 4.5f,
					.alignment = m2::ui::TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 30, .w = 15, .h = 5,
				.variant = TextBlueprint{
					.initial_text = "Cancel",
					.on_action = [](MAYBE const Text& self) -> Action {
						// Check if a user journey is active
						if (auto& user_journey = m2g::Proxy::get_instance().user_journey; user_journey) {
							// Check if BuildJourney is active
							if (std::holds_alternative<BuildJourney>(*user_journey)) {
								// Deliver cancellation to BuildJourney
								std::get<BuildJourney>(*user_journey).signal(PositionOrCancelSignal::create_cancel_signal(true));
							}
						}
						return make_return_action();
					}
				}
			},
			WidgetBlueprint{
				.x = 25, .y = 30, .w = 30, .h = 5,
				.variant = TextBlueprint{
					.initial_text = "OK",
					.on_action = [](MAYBE const Text& self) -> Action {
						// Check if a user journey is active
						if (auto& user_journey = m2g::Proxy::get_instance().user_journey; user_journey) {
							// Check if BuildJourney is active
							if (std::holds_alternative<BuildJourney>(*user_journey)) {
								// Deliver cancellation to BuildJourney
								std::get<BuildJourney>(*user_journey).signal(PositionOrCancelSignal::create_cancel_signal(false));
							}
						}
						return make_return_action();
					}
				}
			}
		}
	};
}
