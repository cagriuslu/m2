#include <cuzn/journeys/ScoutJourney.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>

void execute_scout_journey() {
	if (player_card_count(M2_PLAYER.character()) < 3) {
		M2_LEVEL.display_message("Scout action requires at least 3 cards");
		return;
	}
	if (M2_PLAYER.character().find_items(m2g::pb::ITEM_CATEGORY_WILD_CARD) != M2_PLAYER.character().end_items()) {
		M2_LEVEL.display_message("Scout action is not allowed while holding a wild card");
		return;
	}

	LOG_INFO("Executing scout action");
	if (auto selected_card_0 = ask_for_card_selection()) {
		if (auto selected_card_1 = ask_for_card_selection(*selected_card_0)) {
			if (auto selected_card_2 = ask_for_card_selection(*selected_card_0, *selected_card_1)) {
				auto card_name_0 = M2_GAME.get_named_item(*selected_card_0).in_game_name();
				auto card_name_1 = M2_GAME.get_named_item(*selected_card_1).in_game_name();
				auto card_name_2 = M2_GAME.get_named_item(*selected_card_2).in_game_name();
				if (ask_for_confirmation("Scout using " + card_name_0 + ",", card_name_1 + ", and " + card_name_2 + " cards?", "OK", "Cancel")) {
					LOG_INFO("Scout action confirmed");

					m2g::pb::ClientCommand cc;
					cc.mutable_scout_action()->set_card_0(*selected_card_0);
					cc.mutable_scout_action()->set_card_1(*selected_card_1);
					cc.mutable_scout_action()->set_card_2(*selected_card_2);
					M2_GAME.client_thread().queue_client_command(cc);

					return;
				}
			}
		}
	}
	LOG_INFO("Scout action cancelled");
}
