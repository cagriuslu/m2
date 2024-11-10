#include <cuzn/journeys/LiquidationJourney.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/detail/Liquidate.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <ranges>
#include <numeric>

LiquidationJourney::LiquidationJourney(int money_to_be_paid) : m2::FsmBase<LiquidationJourneyStep, PositionOrCancelSignal>(), _money_to_be_paid(money_to_be_paid) {
	LOG_INFO("Creating LiquidationJourney");
	init(LiquidationJourneyStep::EXPECT_LOCATION);
}

std::optional<LiquidationJourneyStep> LiquidationJourney::handle_signal(const PositionOrCancelSignal& s) {
	static std::initializer_list<std::tuple<
			LiquidationJourneyStep,
			m2::FsmSignalType,
			std::optional<LiquidationJourneyStep> (LiquidationJourney::*)(),
			std::optional<LiquidationJourneyStep> (LiquidationJourney::*)(const PositionOrCancelSignal &)>> handlers = {
			{LiquidationJourneyStep::EXPECT_LOCATION, m2::FsmSignalType::EnterState, &LiquidationJourney::handle_location_enter_signal, nullptr},
			{LiquidationJourneyStep::EXPECT_LOCATION, m2::FsmSignalType::Custom, nullptr, &LiquidationJourney::handle_location_mouse_click_signal},
			{LiquidationJourneyStep::EXPECT_LOCATION, m2::FsmSignalType::ExitState, &LiquidationJourney::handle_location_exit_signal, nullptr},
	};
	return handle_signal_using_handler_map(handlers, *this, s);
}

namespace {
	int sell_return_of_factories(const std::vector<IndustryLocation>& locations) {
		auto half_costs = locations
			| std::views::transform(find_factory_at_location)
			| std::views::transform(m2::to_character_of_object_unsafe)
			| std::views::transform(liquidation_return_of_factory_character);
		return std::accumulate(half_costs.begin(), half_costs.end(), 0);
	}
}

std::optional<LiquidationJourneyStep> LiquidationJourney::handle_location_enter_signal() {
	if (sell_return_of_factories(_selected_locations) < _money_to_be_paid
		&& player_built_factory_count(M2_PLAYER.character()) - _selected_locations.size()) {
		LOG_DEBUG("Expecting liquidation location...");
		M2_LEVEL.disable_hud();
		M2G_PROXY.show_notification("Pick a factory to liquidate");
		return std::nullopt;
	} else {
		m2g::pb::ClientCommand cc;
		for (const auto& loc : _selected_locations) {
			cc.mutable_liquidate_action()->add_locations_to_sell(loc);
		}
		M2_GAME.queue_client_command(cc);
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}
}

std::optional<LiquidationJourneyStep> LiquidationJourney::handle_location_mouse_click_signal(const PositionOrCancelSignal& position_or_cancel) {
	if (position_or_cancel.world_position()) {
		const m2::VecF& world_position = *position_or_cancel.world_position();
		LOG_DEBUG("Received mouse click", world_position);

		// Lookup industry location
		if (auto selected_loc = industry_location_on_position(world_position)) {
			LOG_INFO("Clicked on", m2g::pb::SpriteType_Name(*selected_loc));
			// Search for a factory on the selected location
			if (auto* factory = find_factory_at_location(*selected_loc)) {
				// Check if the owner of the factory is the player
				if (factory->parent_id() == M2_PLAYER.id()) {
					// Ask for confirmation
					auto city_name = M2_GAME.get_named_item(city_of_location(*selected_loc)).in_game_name();
					auto industry = to_industry_of_factory_character(factory->character());
					auto industry_name = M2_GAME.get_named_item(industry).in_game_name();
					auto sell_return = liquidation_return_of_factory_character(factory->character());
					if (ask_for_confirmation("Sell " + industry_name + " in " + city_name, "for £" + std::to_string(sell_return) + "?", "Yes", "No")) {
						_selected_locations.emplace_back(*selected_loc);
						// Disable the drawing of the factory
						factory->graphic().draw = false;
						// Enter the same state
						return LiquidationJourneyStep::EXPECT_LOCATION;
					} else {
						LOG_DEBUG("Confirmation rejected");
					}
				} else {
					M2G_PROXY.show_notification("Selected industry doesn't belong to you");
				}
			} else {
				LOG_DEBUG("Selected position was not on a built industry");
			}
		} else {
			LOG_DEBUG("Selected position was not on an industry");
		}
		return std::nullopt;
	} else {
		throw M2_ERROR("Unexpected signal");
	}
}

std::optional<LiquidationJourneyStep> LiquidationJourney::handle_location_exit_signal() {
	M2_LEVEL.enable_hud();
	return std::nullopt;
}
