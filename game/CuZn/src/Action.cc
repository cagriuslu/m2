#include <cuzn/Action.h>
#include <cuzn/detail/ActionNotification.h>
#include <cuzn/detail/Liquidate.h>
#include <cuzn/journeys/BuildJourney.h>
#include <cuzn/journeys/DevelopJourney.h>
#include <cuzn/journeys/LoanJourney.h>
#include <cuzn/journeys/NetworkJourney.h>
#include <cuzn/journeys/ScoutJourney.h>
#include <cuzn/journeys/SellJourney.h>
#include <cuzn/object/Factory.h>
#include <m2g_ConstantType.pb.h>
#include <m2g_VariableType.pb.h>
#include <m2/Log.h>
#include <m2/Game.h>

m2::void_expected HandleActionWhileLiquidating(m2::Character& turnHolderCharacter, const m2g::pb::TurnBasedClientCommand& clientCommand) {
	if (not clientCommand.has_liquidate_action()) {
		return m2::make_unexpected("Received unexpected command during liquidation");
	}

	if (auto expectFactoriesAndGain = CanPlayerLiquidateFactories(turnHolderCharacter, clientCommand.liquidate_action())) {
		LOG_INFO("Liquidating factories and gaining money", expectFactoriesAndGain->first.size(), expectFactoriesAndGain->second);
		for (const auto* factory : expectFactoriesAndGain->first) {
			M2_LEVEL.objects.Free(factory->GetId());
		}
		turnHolderCharacter.SetVariable(m2g::pb::MONEY, m2::IFE{turnHolderCharacter.GetVariable(m2g::pb::MONEY).GetIntOrZero() + expectFactoriesAndGain->second});
		return {}; // Liquidation successful
	} else {
		return m2::make_unexpected(expectFactoriesAndGain.error());
	}
}

m2::expected<int> HandleActionWhileNotLiquidating(m2::Character& turnHolderCharacter, const m2g::pb::TurnBasedClientCommand& clientCommand, m2g::pb::TurnBasedServerCommand::ActionNotification& actionNotification) {
	if (clientCommand.has_liquidate_action()) {
		return m2::make_unexpected("Received unexpected liquidation command");
	}

	std::pair<m2g::pb::CardType,int> cardToDiscardAndMoneySpent{};
	if (clientCommand.has_build_action()) {
		LOG_INFO("Validating build action");
		if (const auto buildValidation = CanPlayerBuild(turnHolderCharacter, clientCommand.build_action()); not buildValidation) {
			return m2::make_unexpected(buildValidation.error());
		}
		actionNotification.set_notification(GenerateBuildNotification(
				industry_of_industry_tile(clientCommand.build_action().industry_tile()),
				city_of_location(clientCommand.build_action().industry_location())));
		LOG_INFO("Executing build action");
		cardToDiscardAndMoneySpent = ExecuteBuildAction(turnHolderCharacter, clientCommand.build_action());
	} else if (clientCommand.has_network_action()) {
		LOG_INFO("Validating network action");
		if (const auto networkValidation = CanPlayerNetwork(turnHolderCharacter, clientCommand.network_action()); not networkValidation) {
			return m2::make_unexpected(networkValidation.error());
		}
		actionNotification.set_notification(GenerateNetworkNotification(
				cities_from_connection(clientCommand.network_action().connection_1())[0],
				cities_from_connection(clientCommand.network_action().connection_1())[1],
				clientCommand.network_action().connection_2()
					? cities_from_connection(clientCommand.network_action().connection_2())[0] : m2g::pb::NO_CARD,
				clientCommand.network_action().connection_2()
					? cities_from_connection(clientCommand.network_action().connection_2())[1] : m2g::pb::NO_CARD));
		LOG_INFO("Executing network action");
		cardToDiscardAndMoneySpent = ExecuteNetworkAction(turnHolderCharacter, clientCommand.network_action());
	} else if (clientCommand.has_sell_action()) {
		LOG_INFO("Validating sell action");
		if (auto sellValidation = CanPlayerSell(turnHolderCharacter, clientCommand.sell_action()); not sellValidation) {
			return m2::make_unexpected(sellValidation.error());
		}
		actionNotification.set_notification(GenerateSellNotification(
				ToIndustryOfFactoryCharacter(FindFactoryAtLocation(clientCommand.sell_action().industry_location())->GetCharacter()),
				city_of_location(clientCommand.sell_action().industry_location())));
		LOG_INFO("Executing sell action");
		cardToDiscardAndMoneySpent.first = ExecuteSellAction(turnHolderCharacter, clientCommand.sell_action());
	} else if (clientCommand.has_develop_action()) {
		LOG_INFO("Validating develop action");
		if (auto developValidation = CanPlayerDevelop(turnHolderCharacter, clientCommand.develop_action()); not developValidation) {
			return m2::make_unexpected(developValidation.error());
		}
		actionNotification.set_notification(GenerateDevelopNotification(
				industry_of_industry_tile(clientCommand.develop_action().industry_tile_1()),
				clientCommand.develop_action().industry_tile_2()
					? industry_of_industry_tile(clientCommand.develop_action().industry_tile_2()) : m2g::pb::NO_CARD));
		LOG_INFO("Executing develop action");
		cardToDiscardAndMoneySpent = ExecuteDevelopAction(turnHolderCharacter, clientCommand.develop_action());
	} else if (clientCommand.has_loan_action()) {
		LOG_INFO("Validating loan action");
		if (auto loanValidation = CanPlayerLoan(turnHolderCharacter, clientCommand.loan_action()); not loanValidation) {
			return m2::make_unexpected(loanValidation.error());
		}
		actionNotification.set_notification(GenerateLoanNotification());
		LOG_INFO("Executing loan action");
		cardToDiscardAndMoneySpent.first = ExecuteLoanAction(turnHolderCharacter, clientCommand.loan_action());
	} else if (clientCommand.has_scout_action()) {
		LOG_INFO("Validating scout action");
		if (auto scoutValidation = CanPlayerScout(turnHolderCharacter, clientCommand.scout_action()); not scoutValidation) {
			return m2::make_unexpected(scoutValidation.error());
		}
		actionNotification.set_notification(GenerateScoutNotification());
		LOG_INFO("Executing scout action");
		cardToDiscardAndMoneySpent.first = ExecuteScoutAction(turnHolderCharacter, clientCommand.scout_action());
	} else if (clientCommand.has_pass_action()) {
		actionNotification.set_notification(GeneratePassNotification());
		LOG_INFO("Executing pass action");
		cardToDiscardAndMoneySpent.first = clientCommand.pass_action().card();
	}
	auto [cardToDiscard, moneySpent] = cardToDiscardAndMoneySpent;

	// Discard card from player
	if (cardToDiscard) {
		LOG_INFO("Discard card from player", M2_GAME.GetCard(cardToDiscard).in_game_name());
		turnHolderCharacter.RemoveCard(cardToDiscard);
	}
	// Deduct money from player
	LOG_INFO("Deducting money from player", moneySpent);
	turnHolderCharacter.SetVariable(m2g::pb::MONEY, m2::IFE{std::max(turnHolderCharacter.GetVariable(m2g::pb::MONEY).GetIntOrZero() - moneySpent, 0)});

	return moneySpent;
}
