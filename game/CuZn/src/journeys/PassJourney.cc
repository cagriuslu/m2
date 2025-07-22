#include <cuzn/journeys/PassJourney.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Selection.h>
#include <cuzn/ui/Cards.h>
#include <m2/Game.h>

void ExecutePassJourney() {
	LOG_INFO("Executing pass action");
	if (auto selected_card = ask_for_card_selection()) {
		auto card_name = M2_GAME.GetNamedItem(*selected_card).in_game_name();
		if (ask_for_confirmation("Pass turn using " + card_name + " card?", "", "OK", "Cancel")) {
			LOG_INFO("Pass action confirmed");

			m2g::pb::TurnBasedClientCommand cc;
			cc.mutable_pass_action()->set_card(*selected_card);
			M2G_PROXY.SendClientCommandAndWaitForServerUpdate(cc);

			return;
		}
	}
	LOG_INFO("Pass action cancelled");
}
