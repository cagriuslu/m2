#include <cuzn/journeys/ScoutJourney.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <m2/protobuf/Detail.h>

m2::void_expected CanPlayerAttemptToScout(m2::Character& player) {
	// Player needs to hold at least 3 cards to discard
	if (PlayerCardCount(player) < 3) {
		return m2::make_unexpected("Scout action requires at least 3 cards");
	}

	// Player needs to have no other Wild cards
	if (player.find_items(m2g::pb::ITEM_CATEGORY_WILD_CARD) != player.end_items()) {
		return m2::make_unexpected("Scout action is not allowed while already holding a wild card");
	}

	return {};
}

void ExecuteScoutJourney() {
	LOG_INFO("Executing scout action");
	if (auto selected_card_0 = ask_for_card_selection()) {
		if (auto selected_card_1 = ask_for_card_selection(*selected_card_0)) {
			if (auto selected_card_2 = ask_for_card_selection(*selected_card_0, *selected_card_1)) {
				auto card_name_0 = M2_GAME.GetNamedItem(*selected_card_0).in_game_name();
				auto card_name_1 = M2_GAME.GetNamedItem(*selected_card_1).in_game_name();
				auto card_name_2 = M2_GAME.GetNamedItem(*selected_card_2).in_game_name();
				if (ask_for_confirmation("Scout using " + card_name_0 + ",", card_name_1 + ", and " + card_name_2 + " cards?", "OK", "Cancel")) {
					LOG_INFO("Scout action confirmed");

					m2g::pb::ClientCommand cc;
					cc.mutable_scout_action()->set_card_0(*selected_card_0);
					cc.mutable_scout_action()->set_card_1(*selected_card_1);
					cc.mutable_scout_action()->set_card_2(*selected_card_2);
					M2_GAME.QueueClientCommand(cc);

					return;
				}
			}
		}
	}
	LOG_INFO("Scout action cancelled");
}

bool CanPlayerScout(m2::Character& player, const m2g::pb::ClientCommand_ScoutAction& scout_action) {
	// Check if prerequisites are met
	if (auto prerequisite = CanPlayerAttemptToScout(player); not prerequisite) {
		LOG_WARN("Player does not meet scout prerequisites", prerequisite.error());
		return false;
	}

	// Check if the player holds the selected cards
	auto player_card_list = PlayerCards(player);
	for (const auto& card_to_discard : {scout_action.card_0(), scout_action.card_1(), scout_action.card_2()}) {
		auto card_it = std::find(player_card_list.begin(), player_card_list.end(), card_to_discard);
		if (card_it == player_card_list.end()) {
			LOG_WARN("Player does not have the selected card", m2::pb::enum_name(card_to_discard));
			return false;
		}
		player_card_list.erase(card_it);
	}

	return true;
}

Card ExecuteScoutAction(m2::Character& player, const m2g::pb::ClientCommand_ScoutAction& scout_action) {
	auto card_1_it = player.find_items(scout_action.card_1());
	auto card_2_it = player.find_items(scout_action.card_2());
	player.remove_item(card_1_it);
	player.remove_item(card_2_it);
	player.add_named_item(M2_GAME.GetNamedItem(m2g::pb::WILD_INDUSTRY_CARD));
	player.add_named_item(M2_GAME.GetNamedItem(m2g::pb::WILD_LOCATION_CARD));

	return scout_action.card_0();
}
