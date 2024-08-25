#include <cuzn/journeys/LoanJourney.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/detail/Income.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

m2::void_expected can_player_attempt_to_loan(m2::Character& player) {
	if (player_card_count(player) < 1) {
		return m2::make_unexpected("Loan action requires a card");
	}

	// Player income points must not go below -10
	if (player_income_points(player) - 3 < -10) {
		return m2::make_unexpected("Loan action requires at least -7 income points");
	}

	return {};
}

void execute_loan_journey() {
	LOG_INFO("Loan action");
	if (auto selected_card = ask_for_card_selection(); selected_card) {
		auto card_name = M2_GAME.get_named_item(*selected_card).in_game_name();
		if (ask_for_confirmation("Take a loan using ", card_name + " card?", "OK", "Cancel")) {
			LOG_INFO("Loan action confirmed");

			pb::ClientCommand cc;
			cc.mutable_loan_action()->set_card(*selected_card);
			M2_GAME.client_thread().queue_client_command(cc);

			return;
		}
	}
	LOG_INFO("Loan action cancelled");
}

bool can_player_loan(m2::Character& player, const m2g::pb::ClientCommand_LoanAction& loan_action) {
	// Check if prerequisites are met
	if (auto prerequisite = can_player_attempt_to_loan(player); not prerequisite) {
		LOG_WARN("Player does not meet loan prerequisites", prerequisite.error());
		return false;
	}

	// Check if the player holds the selected card
	if (player.find_items(loan_action.card()) == player.end_items()) {
		LOG_WARN("Player does not have the selected card", m2::pb::enum_name(loan_action.card()));
		return false;
	}

	return true;
}

Card execute_loan_action(m2::Character& player, const m2g::pb::ClientCommand_LoanAction& loan_action) {
	auto curr_income_points = player_income_points(player);
	auto curr_income_level = income_level_from_income_points(curr_income_points);
	auto new_income_level = curr_income_level - 3;
	auto new_income_points = highest_income_points_of_level(new_income_level);
	player.set_attribute(pb::INCOME_POINTS, m2::F(new_income_points));
	player.add_resource(pb::MONEY, 30.0f);

	return loan_action.card();
}
