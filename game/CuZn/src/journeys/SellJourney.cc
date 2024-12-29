#include <cuzn/journeys/SellJourney.h>
#include <m2/Log.h>
#include <m2/M2.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <cuzn/ui/Tiles.h>
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
	deinit();
	// Return the reserved resources
	for (auto* object : _reserved_beers) {
		object->character().add_resource(BEER_BARREL_COUNT, 1.0f);
	}
	_reserved_beers.clear();
}

std::optional<SellJourneyStep> SellJourney::handle_signal(const POIOrCancelSignal& s) {
	static std::initializer_list<std::tuple<
			SellJourneyStep,
			FsmSignalType,
			std::optional<SellJourneyStep> (SellJourney::*)(),
			std::optional<SellJourneyStep> (SellJourney::*)(const POIOrCancelSignal &)>> handlers = {
		{SellJourneyStep::INITIAL_STEP, FsmSignalType::EnterState, &SellJourney::handle_initial_enter_signal, nullptr},

		{SellJourneyStep::EXPECT_INDUSTRY_LOCATION, FsmSignalType::EnterState, &SellJourney::handle_industry_location_enter_signal, nullptr},
		{SellJourneyStep::EXPECT_INDUSTRY_LOCATION, FsmSignalType::Custom, nullptr, &SellJourney::handle_industry_location_poi_or_cancel_signal},
		{SellJourneyStep::EXPECT_INDUSTRY_LOCATION, FsmSignalType::ExitState, &SellJourney::handle_industry_location_exit_signal, nullptr},

		{SellJourneyStep::EXPECT_MERCHANT_LOCATION, FsmSignalType::EnterState, &SellJourney::handle_merchant_location_enter_signal, nullptr},
		{SellJourneyStep::EXPECT_MERCHANT_LOCATION, FsmSignalType::Custom, nullptr, &SellJourney::handle_merchant_location_poi_or_cancel_signal},
		{SellJourneyStep::EXPECT_MERCHANT_LOCATION, FsmSignalType::ExitState, &SellJourney::handle_merchant_location_exit_signal, nullptr},

		{SellJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::EnterState, &SellJourney::handle_resource_enter_signal, nullptr},
		{SellJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::Custom, nullptr, &SellJourney::handle_resource_poi_or_cancel_signal},
		{SellJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::ExitState, &SellJourney::handle_resource_exit_signal, nullptr},

		{SellJourneyStep::EXPECT_DEVELOP_BENEFIT_INDUSTRY_TILE, FsmSignalType::EnterState, &SellJourney::handle_develop_benefit_industry_tile_enter_signal, nullptr},

		{SellJourneyStep::EXPECT_CONFIRMATION, FsmSignalType::EnterState, &SellJourney::handle_confirmation_enter_signal, nullptr},
	};
	return handle_signal_using_handler_map(handlers, *this, s);
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
	sub_journey.emplace(sellable_locations, "Pick the industry to sell using right mouse button...");
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_industry_location_poi_or_cancel_signal(const POIOrCancelSignal& s) {
	if (s.poi_or_cancel()) {
		_selected_location = *s.poi_or_cancel();
		return SellJourneyStep::EXPECT_MERCHANT_LOCATION;
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}	
}

std::optional<SellJourneyStep> SellJourney::handle_industry_location_exit_signal() {
	sub_journey.reset();
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_merchant_location_enter_signal() {
	auto merchants_buying = merchants_buying_industry_on_location(_selected_location);
	sub_journey.emplace(merchants_buying, "Pick the merchant to sell to using right mouse button...");
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_merchant_location_poi_or_cancel_signal(const POIOrCancelSignal& s) {
	if (s.poi_or_cancel()) {
		_merchant_location = *s.poi_or_cancel();
		return SellJourneyStep::EXPECT_RESOURCE_SOURCE;
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}
}

std::optional<SellJourneyStep> SellJourney::handle_merchant_location_exit_signal() {
	sub_journey.reset();
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
				M2G_PROXY.show_notification("Beer is required but none is available");
				M2_DEFER(m2g::Proxy::main_journey_deleter);
				return std::nullopt;
			} else {
				sub_journey.emplace(beer_sources, "Pick a beer source using right mouse button...");
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

std::optional<SellJourneyStep> SellJourney::handle_resource_poi_or_cancel_signal(const POIOrCancelSignal& s) {
	if (s.poi_or_cancel()) {	
		auto poi = *s.poi_or_cancel();
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
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}
}

std::optional<SellJourneyStep> SellJourney::handle_resource_exit_signal() {
	if (sub_journey) {
		sub_journey.reset();
	}
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_develop_benefit_industry_tile_enter_signal() {
	// Check if merchant bonus is free develop
	if (is_merchant_benefit_develop(_merchant_location)) {
		// Check if using merchant beer
		if (does_contain_location_from_city(_beer_sources, city_of_location(_merchant_location))) {
			// Ask if player wants to develop
			if (ask_for_confirmation("Merchant offers 1 free develop action.", "Use it?", "Yes", "No")) {
				// Ask for tile selection
				if (auto selected_tile = ask_for_tile_selection()) {
					// Check if tile can be developed
					if (m2::is_equal(M2_GAME.GetNamedItem(*selected_tile).get_attribute(DEVELOPMENT_BAN), 1.0f, 0.001)) {
						M2G_PROXY.show_notification("Selected industry cannot be developed");
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
	auto card_name = M2_GAME.GetNamedItem(_selected_card).in_game_name();
	auto city_name = M2_GAME.GetNamedItem(city_of_location(_selected_location)).in_game_name();
	auto industry = to_industry_of_factory_character(find_factory_at_location(_selected_location)->character());
	auto industry_name = M2_GAME.GetNamedItem(industry).in_game_name();
	if (ask_for_confirmation("Sell " + industry_name + " in " + city_name, "using " + card_name + " card?", "OK", "Cancel")) {
		M2G_PROXY.show_notification("Selling location...");

		m2g::pb::ClientCommand cc;
		cc.mutable_sell_action()->set_card(_selected_card);
		cc.mutable_sell_action()->set_industry_location(_selected_location);
		cc.mutable_sell_action()->set_merchant_location(_merchant_location);
		for (const auto& beer_source : _beer_sources) {
			cc.mutable_sell_action()->add_beer_sources(beer_source);
		}
		cc.mutable_sell_action()->set_merchant_develop_benefit_industry_tile(_merchant_develop_benefit_industry_tile);
		M2_GAME.QueueClientCommand(cc);
	}
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

Industry SellJourney::selected_industry() const {
	return to_industry_of_factory_character(find_factory_at_location(_selected_location)->character());
}

m2::void_expected can_player_sell(m2::Character& player, const m2g::pb::ClientCommand_SellAction& sell_action) {
	auto prerequisite = can_player_attempt_to_sell(player);
	m2_reflect_unexpected(prerequisite);

	// Validate the card
	m2_return_unexpected_message_unless(is_card(sell_action.card()), "Selected card is not a card");
	m2_return_unexpected_message_unless(does_player_hold_card(player, sell_action.card()), "Player does not hold the selected card");

	// Validate the factory
	m2_return_unexpected_message_unless(is_industry_location(sell_action.industry_location()), "Selected location is not an industry location");
	auto* factory = find_factory_at_location(sell_action.industry_location());
	m2_return_unexpected_message_unless(factory, "Selected location does not have a built factory");
	m2_return_unexpected_message_unless(factory->parent_id() == player.owner_id(), "Selected factory does not belong to the player");
	m2_return_unexpected_message_unless(is_sellable_industry(to_industry_of_factory_character(factory->character())), "Selected factory is not sellable");

	// Validate the merchant
	m2_return_unexpected_message_unless(is_merchant_location(sell_action.merchant_location()), "Selected merchant location is not a merchant location");
	auto merchants = merchants_buying_industry_on_location(sell_action.industry_location());
	m2_return_unexpected_message_unless(merchants.contains(sell_action.merchant_location()), "Selected merchant cannot buy the selected industry");

	// Validate the beer sources
	if (auto required_beer_count = required_beer_count_to_sell(sell_action.industry_location())) {
		m2_return_unexpected_message_unless(required_beer_count == sell_action.beer_sources_size(), "Invalid number of beer sources are provided");
		std::vector<m2::Object*> reserved_beers;
		for (const auto& beer_source_i : sell_action.beer_sources()) {
			auto beer_source = static_cast<Location>(beer_source_i);
			auto breweries = find_breweries_with_beer(player, city_of_location(sell_action.industry_location()), sell_action.merchant_location());
			m2_return_unexpected_message_unless(breweries.contains(beer_source), "Selected beer source cannot be used");
			// Reserve the resource
			if (is_industry_location(beer_source)) {
				auto* source_factory = find_factory_at_location(beer_source);
				source_factory->character().remove_resource(BEER_BARREL_COUNT, 1.0f);
				reserved_beers.emplace_back(source_factory);
			} else if (is_merchant_location(beer_source)) {
				auto* source_merchant = find_merchant_at_location(beer_source);
				source_merchant->character().remove_resource(BEER_BARREL_COUNT, 1.0f);
				reserved_beers.emplace_back(source_merchant);
			}
		}
		// Give back the reserved resources
		for (auto* source : reserved_beers) {
			source->character().add_resource(BEER_BARREL_COUNT, 1.0f);
		}
	} else {
		m2_return_unexpected_message_unless(sell_action.beer_sources_size() == 0, "Beer not required but beer sources are provided");
	}

	// Validate merchant develop benefit
	if (auto tile = sell_action.merchant_develop_benefit_industry_tile()) {
		m2_return_unexpected_message_unless(is_merchant_benefit_develop(sell_action.merchant_location()),
			"Selected merchant does not offer develop benefit but a tile to develop has been provided");
		m2_return_unexpected_message_unless(does_contain_location_from_city(sell_action.beer_sources(), city_of_location(sell_action.merchant_location())),
			"Develop benefit tile is provided but merchant beer is not used");
		m2_return_unexpected_message_unless(is_industry_tile(tile),
			"Selected develop benefit tile is not an industry tile");
		m2_return_unexpected_message_unless(get_next_industry_tile_of_category(player, industry_tile_category_of_industry_tile(tile)) == tile,
			"Selected develop benefit tile is not the next tile to develop in the category");
		m2_return_unexpected_message_unless(m2::is_equal(M2_GAME.GetNamedItem(tile).get_attribute(DEVELOPMENT_BAN), 0.0f, 0.001),
			"Selected develop benefit tile cannot be developed");
	}

	return {};
}

Card execute_sell_action(m2::Character& player, const m2g::pb::ClientCommand_SellAction& sell_action) {
	// Assume validation is done

	// Take resources
	for (const auto& beer_source_i : sell_action.beer_sources()) {
		auto beer_source = static_cast<Location>(beer_source_i);
		if (is_industry_location(beer_source)) {
			auto* source_factory = find_factory_at_location(beer_source);
			source_factory->character().remove_resource(BEER_BARREL_COUNT, 1.0f);
		} else if (is_merchant_location(beer_source)) {
			auto* source_merchant = find_merchant_at_location(beer_source);
			source_merchant->character().remove_resource(BEER_BARREL_COUNT, 1.0f);
		}
	}

	sell_factory(find_factory_at_location(sell_action.industry_location())->character());

	// Merchant develop benefit
	if (sell_action.merchant_develop_benefit_industry_tile()) {
		// Take tile from player
		player.remove_item(player.find_items(sell_action.merchant_develop_benefit_industry_tile()));
	}

	flip_exhausted_factories();

	return sell_action.card();
}
