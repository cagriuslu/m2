#include <cuzn/journeys/LoanJourney.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

void execute_loan_journey() {
	LOG_INFO("Loan action");
	if (auto selected_card = ask_for_card_selection(); selected_card) {
		auto card_name = M2_GAME.get_named_item(*selected_card).in_game_name();
		if (ask_for_confirmation("Take a loan using ", card_name + " card?", "OK", "Cancel")) {
			LOG_INFO("Loan action confirmed");
			pb::ClientCommand cc;
			cc.mutable_loan_action()->set_card(*selected_card);
			M2_GAME.client_thread().queue_client_command(cc);
		}
	}
	LOG_INFO("Loan action cancelled");
}
