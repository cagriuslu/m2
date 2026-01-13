#include <cuzn/journeys/SellJourney.h>
#include <m2/Log.h>
#include <m2/M2.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <cuzn/ui/Tiles.h>
#include <cuzn/ui/Selection.h>
#include <m2/Game.h>
#include "cuzn/object/Factory.h"
#include "cuzn/object/Merchant.h"
#include <cuzn/detail/Network.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/ConsumingBeer.h>
#include <cuzn/detail/Income.h>

using namespace m2;
using namespace m2g;
using namespace m2g::pb;

namespace {
	std::set<MerchantLocation> merchants_buying_industry_on_location(IndustryLocation l) {
		auto industry = ToIndustryOfFactoryCharacter(FindFactoryAtLocation(l)->GetCharacter());
		if (not is_sellable_industry(industry)) {
			return {};
		}

		// Check if a merchant connection to an eligible merchant exists
		std::set<MerchantLocation> merchants;
		for (auto reachable_location : ReachableLocationsFromIndustryCity(city_of_location(l))) {
			if (is_merchant_location(reachable_location)) {
				if (can_merchant_buy_sellable_industry(find_merchant_at_location(reachable_location)->GetCharacter(), industry)) {
					merchants.insert(reachable_location);
				}
			}
		}
		return merchants;
	}

	std::set<IndustryLocation> sellable_factory_locations_with_merchant_connection(m2::Character& player) {
		std::set<IndustryLocation> with_merchant_connection;
		// For each sellable location
		for (auto sellable_location : PlayerSellableFactoryLocations(player)) {
			// Check if there's a merchant willing to buy the industry
			if (not merchants_buying_industry_on_location(sellable_location).empty()) {
				with_merchant_connection.insert(sellable_location);
			}
		}
		return with_merchant_connection;
	}

	bool DoesBeerSourcesContainMerchant(const auto& beerSources, MerchantLocation merchantLocation) {
		return std::ranges::find(beerSources, merchantLocation) != beerSources.end();
	}
}

m2::void_expected CanPlayerAttemptToSell(m2::Character& player) {
	if (PlayerCardCount(player) < 1) {
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
		object->GetCharacter().SetVariable(BEER_BARREL_COUNT, IFE{object->GetCharacter().GetVariable(BEER_BARREL_COUNT).GetIntOrZero() + 1});
	}
	_reserved_beers.clear();
}

std::optional<SellJourneyStep> SellJourney::HandleSignal(const POIOrCancelSignal& s) {
	static std::initializer_list<std::tuple<
			SellJourneyStep,
			FsmSignalType,
			std::optional<SellJourneyStep> (SellJourney::*)(),
			std::optional<SellJourneyStep> (SellJourney::*)(const POIOrCancelSignal &)>> handlers = {
		{SellJourneyStep::INITIAL_STEP, FsmSignalType::EnterState, &SellJourney::HandleInitialEnterSignal, nullptr},

		{SellJourneyStep::EXPECT_INDUSTRY_LOCATION, FsmSignalType::EnterState, &SellJourney::HandleIndustryLocationEnterSignal, nullptr},
		{SellJourneyStep::EXPECT_INDUSTRY_LOCATION, FsmSignalType::Custom, nullptr, &SellJourney::HandleIndustryLocationPoiOrCancelSignal},
		{SellJourneyStep::EXPECT_INDUSTRY_LOCATION, FsmSignalType::ExitState, &SellJourney::HandleIndustryLocationExitSignal, nullptr},

		{SellJourneyStep::EXPECT_MERCHANT_LOCATION, FsmSignalType::EnterState, &SellJourney::HandleMerchantLocationEnterSignal, nullptr},
		{SellJourneyStep::EXPECT_MERCHANT_LOCATION, FsmSignalType::Custom, nullptr, &SellJourney::HandleMerchantLocationPoiOrCancelSignal},
		{SellJourneyStep::EXPECT_MERCHANT_LOCATION, FsmSignalType::ExitState, &SellJourney::HandleMerchantLocationExitSignal, nullptr},

		{SellJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::EnterState, &SellJourney::HandleResourceEnterSignal, nullptr},
		{SellJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::Custom, nullptr, &SellJourney::HandleResourcePoiOrCancelSignal},
		{SellJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::ExitState, &SellJourney::HandleResourceExitSignal, nullptr},

		{SellJourneyStep::EXPECT_DEVELOP_BENEFIT_INDUSTRY_TILE, FsmSignalType::EnterState, &SellJourney::HandleDevelopBenefitIndustryTileEnterSignal, nullptr},

		{SellJourneyStep::EXPECT_CONFIRMATION, FsmSignalType::EnterState, &SellJourney::HandleConfirmationEnterSignal, nullptr},
	};
	return handle_signal_using_handler_map(handlers, *this, s);
}

std::optional<SellJourneyStep> SellJourney::HandleInitialEnterSignal() {
	if (auto selected_card = ask_for_card_selection(); selected_card) {
		_selected_card = *selected_card;
		return SellJourneyStep::EXPECT_INDUSTRY_LOCATION;
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}
}

std::optional<SellJourneyStep> SellJourney::HandleIndustryLocationEnterSignal() {
	auto sellable_locations = sellable_factory_locations_with_merchant_connection(M2_PLAYER.GetCharacter());
	sub_journey.emplace(sellable_locations, "Pick the industry to sell using right mouse button...");
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::HandleIndustryLocationPoiOrCancelSignal(const POIOrCancelSignal& s) {
	if (s.poi()) {
		_selected_location = *s.poi();
		return SellJourneyStep::EXPECT_MERCHANT_LOCATION;
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}	
}

std::optional<SellJourneyStep> SellJourney::HandleIndustryLocationExitSignal() {
	sub_journey.reset();
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::HandleMerchantLocationEnterSignal() {
	auto merchants_buying = merchants_buying_industry_on_location(_selected_location);
	sub_journey.emplace(merchants_buying, "Pick the merchant to sell to using right mouse button...");
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::HandleMerchantLocationPoiOrCancelSignal(const POIOrCancelSignal& s) {
	if (s.poi()) {
		_merchant_location = *s.poi();
		return SellJourneyStep::EXPECT_RESOURCE_SOURCE;
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}
}

std::optional<SellJourneyStep> SellJourney::HandleMerchantLocationExitSignal() {
	sub_journey.reset();
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::HandleResourceEnterSignal() {
	if (auto beer_count = RequiredBeerCountToSell(_selected_location)) {
		// On first entry, create empty entries in _beer_sources for every required beer
		if (_beer_sources.empty()) {
			_beer_sources.insert(_beer_sources.end(), beer_count, NO_SPRITE);
		}
		// Check next beer requiring sourcing
		if (auto unspecified_resource = std::find(_beer_sources.begin(), _beer_sources.end(), NO_SPRITE);
			unspecified_resource != _beer_sources.end()) {
			auto beer_sources = find_breweries_with_beer(M2_PLAYER.GetCharacter(), city_of_location(_selected_location), _merchant_location);
			if (beer_sources.empty()) {
				M2_LEVEL.ShowMessage("Beer is required but none is available", 8.0f);
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

std::optional<SellJourneyStep> SellJourney::HandleResourcePoiOrCancelSignal(const POIOrCancelSignal& s) {
	if (s.poi()) {
		auto poi = *s.poi();
		// Reserve the resource
		if (is_industry_location(poi)) {
			auto* factory = FindFactoryAtLocation(poi);
			factory->GetCharacter().SetVariable(BEER_BARREL_COUNT, IFE{std::max(factory->GetCharacter().GetVariable(BEER_BARREL_COUNT).GetIntOrZero() - 1, 0)});
			_reserved_beers.emplace_back(factory);
		} else if (is_merchant_location(poi)) {
			auto* merchant = find_merchant_at_location(poi);
			merchant->GetCharacter().SetVariable(BEER_BARREL_COUNT, IFE{std::max(merchant->GetCharacter().GetVariable(BEER_BARREL_COUNT).GetIntOrZero() - 1, 0)});
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

std::optional<SellJourneyStep> SellJourney::HandleResourceExitSignal() {
	if (sub_journey) {
		sub_journey.reset();
	}
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::HandleDevelopBenefitIndustryTileEnterSignal() {
	// Check if merchant bonus is free develop
	if (DoesMerchantHasDevelopBenefit(_merchant_location)) {
		// Check if using merchant beer
		if (DoesBeerSourcesContainMerchant(_beer_sources, _merchant_location)) {
			// Ask if player wants to develop
			if (ask_for_confirmation("Merchant offers 1 free develop action.", "Use it?", "Yes", "No")) {
				// Ask for tile selection
				if (const auto selected_tile = ask_for_tile_selection()) {
					// Check if tile can be developed
					if (M2_GAME.GetNamedItem(*selected_tile).GetConstant(DEVELOPMENT_BAN)) {
						M2_LEVEL.ShowMessage("Selected industry cannot be developed", 8.0f);
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

std::optional<SellJourneyStep> SellJourney::HandleConfirmationEnterSignal() {
	const auto card_name = M2_GAME.GetNamedItem(_selected_card).in_game_name();
	const auto city_name = M2_GAME.GetNamedItem(city_of_location(_selected_location)).in_game_name();
	const auto industry = ToIndustryOfFactoryCharacter(FindFactoryAtLocation(_selected_location)->GetCharacter());
	const auto industry_name = M2_GAME.GetNamedItem(industry).in_game_name();
	if (ask_for_confirmation("Sell " + industry_name + " in " + city_name, "using " + card_name + " card?", "OK", "Cancel")) {
		m2g::pb::TurnBasedClientCommand cc;
		cc.mutable_sell_action()->set_card(_selected_card);
		cc.mutable_sell_action()->set_industry_location(_selected_location);
		cc.mutable_sell_action()->set_merchant_location(_merchant_location);
		for (const auto& beer_source : _beer_sources) {
			cc.mutable_sell_action()->add_beer_sources(beer_source);
		}
		cc.mutable_sell_action()->set_merchant_develop_benefit_industry_tile(_merchant_develop_benefit_industry_tile);
		M2G_PROXY.SendClientCommandAndWaitForServerUpdate(cc);
	}
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

Industry SellJourney::selected_industry() const {
	return ToIndustryOfFactoryCharacter(FindFactoryAtLocation(_selected_location)->GetCharacter());
}

m2::void_expected CanPlayerSell(m2::Character& player, const m2g::pb::TurnBasedClientCommand_SellAction& sell_action) {
	auto prerequisite = CanPlayerAttemptToSell(player);
	m2ReflectUnexpected(prerequisite);

	// Validate the card
	m2ReturnUnexpectedUnless(is_card(sell_action.card()), "Selected card is not a card");
	m2ReturnUnexpectedUnless(PlayerHasCard(player, sell_action.card()), "Player does not hold the selected card");

	// Validate the factory
	m2ReturnUnexpectedUnless(is_industry_location(sell_action.industry_location()), "Selected location is not an industry location");
	auto* factory = FindFactoryAtLocation(sell_action.industry_location());
	m2ReturnUnexpectedUnless(factory, "Selected location does not have a built factory");
	m2ReturnUnexpectedUnless(factory->GetParentId() == player.OwnerId(), "Selected factory does not belong to the player");
	m2ReturnUnexpectedUnless(is_sellable_industry(ToIndustryOfFactoryCharacter(factory->GetCharacter())), "Selected factory is not sellable");

	// Validate the merchant
	m2ReturnUnexpectedUnless(is_merchant_location(sell_action.merchant_location()), "Selected merchant location is not a merchant location");
	auto merchants = merchants_buying_industry_on_location(sell_action.industry_location());
	m2ReturnUnexpectedUnless(merchants.contains(sell_action.merchant_location()), "Selected merchant cannot buy the selected industry");

	// Validate the beer sources
	if (auto required_beer_count = RequiredBeerCountToSell(sell_action.industry_location())) {
		m2ReturnUnexpectedUnless(required_beer_count == sell_action.beer_sources_size(), "Invalid number of beer sources are provided");
		std::vector<m2::Object*> reserved_beers;
		for (const auto& beer_source_i : sell_action.beer_sources()) {
			auto beer_source = static_cast<Location>(beer_source_i);
			auto breweries = find_breweries_with_beer(player, city_of_location(sell_action.industry_location()), sell_action.merchant_location());
			m2ReturnUnexpectedUnless(breweries.contains(beer_source), "Selected beer source cannot be used");
			// Reserve the resource
			if (is_industry_location(beer_source)) {
				auto* source_factory = FindFactoryAtLocation(beer_source);
				source_factory->GetCharacter().SetVariable(BEER_BARREL_COUNT, IFE{std::max(source_factory->GetCharacter().GetVariable(BEER_BARREL_COUNT).GetIntOrZero() - 1, 0)});
				reserved_beers.emplace_back(source_factory);
			} else if (is_merchant_location(beer_source)) {
				auto* source_merchant = find_merchant_at_location(beer_source);
				source_merchant->GetCharacter().SetVariable(BEER_BARREL_COUNT, IFE{std::max(source_merchant->GetCharacter().GetVariable(BEER_BARREL_COUNT).GetIntOrZero() - 1, 0)});
				reserved_beers.emplace_back(source_merchant);
			}
		}
		// Give back the reserved resources
		for (auto* source : reserved_beers) {
			source->GetCharacter().SetVariable(BEER_BARREL_COUNT, IFE{source->GetCharacter().GetVariable(BEER_BARREL_COUNT).GetIntOrZero() + 1});
		}
	} else {
		m2ReturnUnexpectedUnless(sell_action.beer_sources_size() == 0, "Beer not required but beer sources are provided");
	}

	// Validate merchant develop benefit
	if (const auto tile = sell_action.merchant_develop_benefit_industry_tile()) {
		m2ReturnUnexpectedUnless(DoesMerchantHasDevelopBenefit(sell_action.merchant_location()),
			"Selected merchant does not offer develop benefit but a tile to develop has been provided");
		m2ReturnUnexpectedUnless(DoesBeerSourcesContainMerchant(sell_action.beer_sources(), sell_action.merchant_location()),
			"Develop benefit tile is provided but merchant beer is not used");
		m2ReturnUnexpectedUnless(is_industry_tile(tile),
			"Selected develop benefit tile is not an industry tile");
		m2ReturnUnexpectedUnless(PlayerNextIndustryTileOfCategory(player, industry_tile_category_of_industry_tile(tile)) == tile,
			"Selected develop benefit tile is not the next tile to develop in the category");
		m2ReturnUnexpectedUnless(M2_GAME.GetNamedItem(tile).GetConstant(DEVELOPMENT_BAN).GetIntOrZero() == 0,
			"Selected develop benefit tile cannot be developed");
	}

	return {};
}

Card ExecuteSellAction(m2::Character& player, const m2g::pb::TurnBasedClientCommand_SellAction& sell_action) {
	// Assume validation is done

	// Take resources
	for (const auto& beer_source_i : sell_action.beer_sources()) {
		auto beer_source = static_cast<Location>(beer_source_i);
		if (is_industry_location(beer_source)) {
			auto* source_factory = FindFactoryAtLocation(beer_source);
			source_factory->GetCharacter().SetVariable(BEER_BARREL_COUNT, IFE{std::max(source_factory->GetCharacter().GetVariable(BEER_BARREL_COUNT).GetIntOrZero() - 1, 0)});
		} else if (is_merchant_location(beer_source)) {
			auto* source_merchant = find_merchant_at_location(beer_source);
			source_merchant->GetCharacter().SetVariable(BEER_BARREL_COUNT, IFE{std::max(source_merchant->GetCharacter().GetVariable(BEER_BARREL_COUNT).GetIntOrZero() - 1, 0)});
		}
	}

	SellFactory(FindFactoryAtLocation(sell_action.industry_location())->GetCharacter());

	// Merchant develop benefit
	if (sell_action.merchant_develop_benefit_industry_tile()) {
		// Take tile from player
		player.RemoveItem(player.FindItems(sell_action.merchant_develop_benefit_industry_tile()));
	}
	if (DoesBeerSourcesContainMerchant(sell_action.beer_sources(), sell_action.merchant_location())) {
		if (const auto incomePointsBenefit = MerchantIncomePointsBenefit(sell_action.merchant_location())) {
			LOG_INFO("Player income points benefit", incomePointsBenefit);
			player.SetVariable(INCOME_POINTS, IFE{ClampIncomePoints(PlayerIncomePoints(player) + incomePointsBenefit)});
		}
		if (const auto victoryPointsBenefit = MerchantVictoryPointsBenefit(sell_action.merchant_location())) {
			LOG_INFO("Player victory points benefit", victoryPointsBenefit);
			player.SetVariable(VICTORY_POINTS, IFE{player.GetVariable(VICTORY_POINTS).GetIntOrZero() + victoryPointsBenefit});
		}
		if (const auto moneyPointsBenefit = MerchantMoneyBenefit(sell_action.merchant_location())) {
			LOG_INFO("Player income points benefit", moneyPointsBenefit);
			player.SetVariable(MONEY, IFE{player.GetVariable(MONEY).GetIntOrZero() + moneyPointsBenefit});
		}
	}

	FlipExhaustedFactories();

	return sell_action.card();
}
