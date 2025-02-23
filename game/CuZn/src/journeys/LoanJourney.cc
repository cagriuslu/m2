#include <cuzn/journeys/LoanJourney.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Selection.h>
#include <cuzn/ui/Cards.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/detail/Income.h>

using namespace m2::widget;
using namespace m2g;
using namespace m2g::pb;

m2::void_expected CanPlayerAttemptToLoan(m2::Character& player) {
	if (PlayerCardCount(player) < 1) {
		return m2::make_unexpected("Loan action requires a card");
	}

	// Player income points must not go below -10
	if (PlayerIncomePoints(player) - 3 < -10) {
		return m2::make_unexpected("Loan action requires at least -7 income points");
	}

	return {};
}

void ExecuteLoanJourney() {
	LOG_INFO("Loan action");
	if (auto selected_card = ask_for_card_selection(); selected_card) {
		auto card_name = M2_GAME.GetNamedItem(*selected_card).in_game_name();
		if (ask_for_confirmation("Take a loan using ", card_name + " card?", "OK", "Cancel")) {
			LOG_INFO("Loan action confirmed");

			pb::ClientCommand cc;
			cc.mutable_loan_action()->set_card(*selected_card);
			M2G_PROXY.SendClientCommandAndWaitForServerUpdate(cc);

			return;
		}
	}
	LOG_INFO("Loan action cancelled");
}

m2::void_expected CanPlayerLoan(m2::Character& player, const m2g::pb::ClientCommand_LoanAction& loan_action) {
	// Check if prerequisites are met
	if (auto prerequisite = CanPlayerAttemptToLoan(player); not prerequisite) {
		return m2::make_unexpected(prerequisite.error());
	}

	// Check if the player holds the selected card
	if (player.FindItems(loan_action.card()) == player.EndItems()) {
		return m2::make_unexpected("Player does not have the selected card: " + m2::pb::enum_name(loan_action.card()));
	}

	return {};
}

Card ExecuteLoanAction(m2::Character& player, const ClientCommand_LoanAction& loan_action) {
	const auto currIncomePoints = PlayerIncomePoints(player);
	const auto currIncomeLevel = IncomeLevelFromIncomePoints(currIncomePoints);
	const auto newIncomeLevel = ClampIncomeLevel(currIncomeLevel - 3);
	const auto newIncomePoints = HighestIncomePointsOfLevel(newIncomeLevel);
	player.SetAttribute(INCOME_POINTS, m2::F(ClampIncomePoints(newIncomePoints)));
	player.AddResource(MONEY, 30.0f);
	return loan_action.card();
}
