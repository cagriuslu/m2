#include <cuzn/journeys/SellJourney.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include "cuzn/object/Factory.h"
#include "cuzn/object/Merchant.h"
#include <cuzn/detail/Network.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/ConsumingBeer.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

namespace {
	std::set<MerchantLocation> merchants_buying_industry_on_location(IndustryLocation l) {
		auto industry = to_industry_of_factory_character(find_factory_at_location(l)->character());
		if (not is_sellable_industry(industry)) {
			return {};
		}

		// Check if a merchant connection to an eligible merchant exists
		std::set<MerchantLocation> merchants;
		for (auto reachable_location : reachable_locations_from_industry_city(city_of_location(l))) {
			if (is_merchant_location(reachable_location)) {
				if (can_merchant_buy_sellable_industry(find_merchant_at_location(reachable_location)->character(), industry)) {
					merchants.insert(reachable_location);
				}
			}
		}
		return merchants;
	}

	std::set<IndustryLocation> sellable_factory_locations_with_merchant_connection(m2::Character& player) {
		std::set<IndustryLocation> with_merchant_connection;
		// For each sellable location
		for (auto sellable_location : player_sellable_factory_locations(player)) {
			// Check if there's a merchant willing to buy the industry
			if (not merchants_buying_industry_on_location(sellable_location).empty()) {
				with_merchant_connection.insert(sellable_location);
			}
		}
		return with_merchant_connection;
	}
}

m2::void_expected can_player_attempt_to_sell(m2::Character& player) {
	if (player_card_count(player) < 1) {
		return m2::make_unexpected("Sell action requires a card");
	}

	if (sellable_factory_locations_with_merchant_connection(player).empty()) {
		return m2::make_unexpected("Sell action requires a sellable industry with a connection to merchant");
	}

	return {};
}

SellJourney::SellJourney() : m2::FsmBase<SellJourneyStep, POIOrCancelSignal>() {
	DEBUG_FN();
	init(SellJourneyStep::INITIAL_STEP);
}

SellJourney::~SellJourney() {
	// Return the reserved resources
	for (auto* object : _reserved_beers) {
		object->character().add_resource(BEER_BARREL_COUNT, 1.0f);
	}
	_reserved_beers.clear();
}

std::optional<SellJourneyStep> SellJourney::handle_signal(const POIOrCancelSignal& s) {
	struct Handlers {
		std::optional<SellJourneyStep> (SellJourney::*enter_handler)();
		std::optional<SellJourneyStep> (SellJourney::*location_handler)(Location);
		std::optional<SellJourneyStep> (SellJourney::*cancel_handler)();
		std::optional<SellJourneyStep> (SellJourney::*exit_handler)();
	};
	std::initializer_list<std::pair<SellJourneyStep, Handlers>> handlers = {
		{SellJourneyStep::INITIAL_STEP, {
			&SellJourney::handle_initial_enter_signal,
			nullptr,
			nullptr,
			nullptr
		}},
		{SellJourneyStep::EXPECT_INDUSTRY_LOCATION, {
			&SellJourney::handle_industry_location_enter_signal,
			&SellJourney::handle_industry_location_poi_signal,
			&SellJourney::handle_industry_location_cancel_signal,
			&SellJourney::handle_industry_location_exit_signal
		}},
		{SellJourneyStep::EXPECT_MERCHANT_LOCATION, {
			&SellJourney::handle_merchant_location_enter_signal,
			&SellJourney::handle_merchant_location_poi_signal,
			&SellJourney::handle_merchant_location_cancel_signal,
			&SellJourney::handle_merchant_location_exit_signal
		}},
		{SellJourneyStep::EXPECT_RESOURCE_SOURCE, {
			&SellJourney::handle_resource_enter_signal,
			&SellJourney::handle_resource_poi_signal,
			&SellJourney::handle_resource_cancel_signal,
			&SellJourney::handle_resource_exit_signal
		}},
		{SellJourneyStep::EXPECT_DEVELOP_BENEFIT_INDUSTRY_TILE, {
			&SellJourney::handle_develop_benefit_industry_tile_enter_signal,
			nullptr,
			nullptr,
			nullptr
		}},
		{SellJourneyStep::EXPECT_CONFIRMATION, {
			&SellJourney::handle_confirmation_enter_signal,
			nullptr,
			nullptr,
			nullptr
		}},
	};

	auto handler_it = std::ranges::find_if(handlers, [this](const auto& pair) { return pair.first == state(); });
	switch (s.type()) {
		case FsmSignalType::EnterState:
			if (handler_it->second.enter_handler) {
				return std::invoke(handler_it->second.enter_handler, this);
			}
			break;
		case FsmSignalType::Custom:
			if (auto poi = s.poi_or_cancel(); poi && handler_it->second.location_handler) {
				return std::invoke(handler_it->second.location_handler, this, *poi);
			} else if (handler_it->second.cancel_handler) {
				return std::invoke(handler_it->second.cancel_handler, this);
			}
			break;
		case FsmSignalType::ExitState:
			if (handler_it->second.exit_handler) {
				return std::invoke(handler_it->second.exit_handler, this);
			}
			break;
		default:
			throw M2_ERROR("Unexpected signal");
	}
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_initial_enter_signal() {
	if (auto selected_card = ask_for_card_selection(); selected_card) {
		_selected_card = *selected_card;
		return SellJourneyStep::EXPECT_INDUSTRY_LOCATION;
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}
}

std::optional<SellJourneyStep> SellJourney::handle_industry_location_enter_signal() {
	auto sellable_locations = sellable_factory_locations_with_merchant_connection(M2_PLAYER.character());
	M2G_PROXY.sub_journey.emplace(sellable_locations, "Pick the industry to sell...");
	M2G_PROXY.sub_journey->init(POISelectionJourneyStep::INITIAL_STEP);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_industry_location_poi_signal(POI poi) {
	_selected_location = poi;
	return SellJourneyStep::EXPECT_MERCHANT_LOCATION;
}

std::optional<SellJourneyStep> SellJourney::handle_industry_location_cancel_signal() {
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_industry_location_exit_signal() {
	M2G_PROXY.sub_journey->deinit();
	M2G_PROXY.sub_journey.reset();
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_merchant_location_enter_signal() {
	auto merchants_buying = merchants_buying_industry_on_location(_selected_location);
	M2G_PROXY.sub_journey.emplace(merchants_buying, "Pick the merchant to sell to...");
	M2G_PROXY.sub_journey->init(POISelectionJourneyStep::INITIAL_STEP);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_merchant_location_poi_signal(POI poi) {
	_merchant_location = poi;
	return SellJourneyStep::EXPECT_RESOURCE_SOURCE;
}

std::optional<SellJourneyStep> SellJourney::handle_merchant_location_cancel_signal() {
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_merchant_location_exit_signal() {
	M2G_PROXY.sub_journey->deinit();
	M2G_PROXY.sub_journey.reset();
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_resource_enter_signal() {
	if (auto beer_count = required_beer_count_to_sell(_selected_location)) {
		// On first entry, create empty entries in _beer_sources for every required beer
		if (_beer_sources.empty()) {
			_beer_sources.insert(_beer_sources.end(), beer_count, NO_SPRITE);
		}
		// Check next beer requiring sourcing
		if (auto unspecified_resource = std::find(_beer_sources.begin(), _beer_sources.end(), NO_SPRITE);
			unspecified_resource != _beer_sources.end()) {
			auto beer_sources = find_breweries_with_beer(M2_PLAYER.character(), city_of_location(_selected_location), _merchant_location);
			if (beer_sources.empty()) {
				M2_LEVEL.display_message("Beer is required but none is available");
				M2_DEFER(m2g::Proxy::main_journey_deleter);
				return std::nullopt;
			} else {
				M2G_PROXY.sub_journey.emplace(beer_sources, "Pick a beer source...");
				M2G_PROXY.sub_journey->init(POISelectionJourneyStep::INITIAL_STEP);
				return std::nullopt;
			}
		} else {
			// All beers are sourced
			return SellJourneyStep::EXPECT_DEVELOP_BENEFIT_INDUSTRY_TILE;
		}
	} else {
		// Beer not required
		return SellJourneyStep::EXPECT_DEVELOP_BENEFIT_INDUSTRY_TILE;
	}
}

std::optional<SellJourneyStep> SellJourney::handle_resource_poi_signal(POI poi) {
	// Reserve the resource
	if (is_industry_location(poi)) {
		auto* factory = find_factory_at_location(poi);
		factory->character().remove_resource(BEER_BARREL_COUNT, 1.0f);
		_reserved_beers.emplace_back(factory);
	} else if (is_merchant_location(poi)) {
		auto* merchant = find_merchant_at_location(poi);
		merchant->character().remove_resource(BEER_BARREL_COUNT, 1.0f);
		_reserved_beers.emplace_back(merchant);
	}
	// Specify source
	*std::find(_beer_sources.begin(), _beer_sources.end(), NO_SPRITE) = poi;
	// Re-enter resource selection
	return SellJourneyStep::EXPECT_RESOURCE_SOURCE;
}

std::optional<SellJourneyStep> SellJourney::handle_resource_cancel_signal() {
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_resource_exit_signal() {
	if (M2G_PROXY.sub_journey) {
		M2G_PROXY.sub_journey->deinit();
		M2G_PROXY.sub_journey.reset();
	}
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_develop_benefit_industry_tile_enter_signal() {
	// Check if merchant bonus is free develop
	auto merchant_city = city_of_location(_merchant_location);
	if (M2_GAME.get_named_item(merchant_city).has_attribute(MERCHANT_BONUS_DEVELOP)) {
		// Check if using merchant beer
		if (std::any_of(_beer_sources.begin(), _beer_sources.end(),
			[merchant_city](Location l) { return city_of_location(l) == merchant_city; })) {
			// Ask if player wants to develop
			if (ask_for_confirmation("Merchant offers 1 free develop action.", "Use it?", "Yes", "No")) {
				// Ask for tile selection
				if (auto selected_tile = ask_for_tile_selection()) {
					// Check if tile can be developed
					if (m2::is_equal(M2_GAME.get_named_item(*selected_tile).get_attribute(DEVELOPMENT_BAN), 1.0f, 0.001)) {
						M2_LEVEL.display_message("Selected industry cannot be developed");
						M2_DEFER(m2g::Proxy::main_journey_deleter);
						return std::nullopt;
					}
					// Record for later
					_merchant_develop_benefit_industry_tile = *selected_tile;
				}
			}
		}
	}
	return SellJourneyStep::EXPECT_CONFIRMATION;
}

std::optional<SellJourneyStep> SellJourney::handle_confirmation_enter_signal() {
	auto card_name = M2_GAME.get_named_item(_selected_card).in_game_name();
	auto city_name = M2_GAME.get_named_item(city_of_location(_selected_location)).in_game_name();
	auto industry = to_industry_of_factory_character(find_factory_at_location(_selected_location)->character());
	auto industry_name = M2_GAME.get_named_item(industry).in_game_name();
	if (ask_for_confirmation("Sell " + industry_name + " in " + city_name, "using " + card_name + " card?", "OK", "Cancel")) {
		M2_LEVEL.display_message("Selling location...");

		m2g::pb::ClientCommand cc;
		cc.mutable_sell_action()->set_card(_selected_card);
		cc.mutable_sell_action()->set_industry_location(_selected_location);
		cc.mutable_sell_action()->set_merchant_location(_merchant_location);
		for (const auto& beer_source : _beer_sources) {
			cc.mutable_sell_action()->add_beer_sources(beer_source);
		}
		cc.mutable_sell_action()->set_merchant_develop_benefit_industry_tile(_merchant_develop_benefit_industry_tile);
		M2_GAME.client_thread().queue_client_command(cc);
	}
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

Industry SellJourney::selected_industry() const {
	return to_industry_of_factory_character(find_factory_at_location(_selected_location)->character());
}

bool can_player_sell(m2::Character& player, const m2g::pb::ClientCommand_SellAction& sell_action) {
	// Check if prerequisites are met
	if (auto prerequisite = can_player_attempt_to_sell(player); not prerequisite) {
		LOG_WARN("Player does not meet sell prerequisites", prerequisite.error());
		return false;
	}

	// Check if the player holds the selected card
	if (not is_card(sell_action.card())) {
		LOG_WARN("Selected card is not a card");
		return false;
	}
	if (player.find_items(sell_action.card()) == player.end_items()) {
		LOG_WARN("Player does not have the selected card");
		return false;
	}

	// TODO

	return true;
}
