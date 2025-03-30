#include <cuzn/journeys/LiquidationJourney.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/detail/Liquidate.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <ranges>
#include <numeric>

LiquidationJourney::LiquidationJourney(int money_to_be_paid) : m2::FsmBase<LiquidationJourneyStep, POIOrCancelSignal>(), _money_to_be_paid(money_to_be_paid) {
	LOG_INFO("Creating LiquidationJourney");
	init(LiquidationJourneyStep::EXPECT_LOCATION);
}

std::optional<LiquidationJourneyStep> LiquidationJourney::HandleSignal(const POIOrCancelSignal& s) {
	static std::initializer_list<std::tuple<
			LiquidationJourneyStep,
			m2::FsmSignalType,
			std::optional<LiquidationJourneyStep> (LiquidationJourney::*)(),
			std::optional<LiquidationJourneyStep> (LiquidationJourney::*)(const POIOrCancelSignal &)>> handlers = {
			{LiquidationJourneyStep::EXPECT_LOCATION, m2::FsmSignalType::EnterState, &LiquidationJourney::HandleLocationEnterSignal, nullptr},
			{LiquidationJourneyStep::EXPECT_LOCATION, m2::FsmSignalType::Custom, nullptr, &LiquidationJourney::HandleLocationMouseClickSignal},
			{LiquidationJourneyStep::EXPECT_LOCATION, m2::FsmSignalType::ExitState, &LiquidationJourney::HandleLocationExitSignal, nullptr},
	};
	return handle_signal_using_handler_map(handlers, *this, s);
}

namespace {
	int sell_return_of_factories(const std::vector<IndustryLocation>& locations) {
		auto half_costs = locations
			| std::views::transform(FindFactoryAtLocation)
			| std::views::transform(m2::to_character_of_object_unsafe)
			| std::views::transform(LiquidationReturnOfFactoryCharacter);
		return std::accumulate(half_costs.begin(), half_costs.end(), 0);
	}
}

std::optional<LiquidationJourneyStep> LiquidationJourney::HandleLocationEnterSignal() {
	if (sell_return_of_factories(_selected_locations) < _money_to_be_paid) {
		auto player_factories = PlayerBuiltFactoryLocations(M2_PLAYER.GetCharacter());
		std::set<IndustryLocation> liquidateable_factories;
		// Remove selected locations from player's factories
		std::set_difference(player_factories.begin(), player_factories.end(),
				_selected_locations.begin(), _selected_locations.end(),
				std::inserter(liquidateable_factories, liquidateable_factories.end()));
		// If there are any liquidate-able factories left
		if (not liquidateable_factories.empty()) {
			sub_journey.emplace(liquidateable_factories, "Right click on a factory to be sold to pay back loans...",
					false);
			return std::nullopt;
		}
	}
	// Otherwise, return from journey. Selected locations will be sent by the exit handler
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

std::optional<LiquidationJourneyStep> LiquidationJourney::HandleLocationMouseClickSignal(const POIOrCancelSignal& poi_or_cancel) {
	// Cancellation is not allowed, assume POI is received

	auto selected_location = *poi_or_cancel.poi();
	LOG_INFO("Location selected for liquidation", m2g::pb::SpriteType_Name(selected_location));
	_selected_locations.emplace_back(selected_location);
	// Destroy the POI selection journey before the next iteration
	sub_journey.reset();
	// Reenter the same state
	return LiquidationJourneyStep::EXPECT_LOCATION;
}

std::optional<LiquidationJourneyStep> LiquidationJourney::HandleLocationExitSignal() {
	// Send the Liquidate action
	m2g::pb::ClientCommand cc;
	for (const auto& loc : _selected_locations) {
		cc.mutable_liquidate_action()->add_locations_to_sell(loc);
	}
	M2G_PROXY.SendClientCommandAndWaitForServerUpdate(cc);
	return std::nullopt;
}
