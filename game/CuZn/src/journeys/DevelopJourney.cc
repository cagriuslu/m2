#include <cuzn/journeys/DevelopJourney.h>
#include <m2/Log.h>
#include "m2/Game.h"
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <cuzn/ui/Tiles.h>
#include <cuzn/ConsumingIron.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/GameStateTracker.h>

using namespace m2;
using namespace m2g;
using namespace m2g::pb;

m2::void_expected CanPlayerAttemptToDevelop(m2::Character& player) {
	if (PlayerCardCount(player) < 1) {
		return m2::make_unexpected("Develop action requires a card");
	}

	if (PlayerIndustryTileCount(player) < 1) {
		return m2::make_unexpected("Develop action requires an industry tile");
	}

	return {};
}

DevelopJourney::DevelopJourney() : FsmBase() {
	DEBUG_FN();
	init(DevelopJourneyStep::INITIAL_STEP);
}

DevelopJourney::~DevelopJourney() {
	deinit();
	// Return the reserved resources
	if (_reserved_source_1) {
		_reserved_source_1->character().add_resource(IRON_CUBE_COUNT, 1.0f);
		_reserved_source_1 = nullptr;
	}
	if (_reserved_source_2) {
		_reserved_source_2->character().add_resource(IRON_CUBE_COUNT, 1.0f);
		_reserved_source_2 = nullptr;
	}
}

std::optional<DevelopJourneyStep> DevelopJourney::HandleSignal(const POIOrCancelSignal& s) {
	static std::initializer_list<std::tuple<
			DevelopJourneyStep,
			FsmSignalType,
			std::optional<DevelopJourneyStep> (DevelopJourney::*)(),
			std::optional<DevelopJourneyStep> (DevelopJourney::*)(const POIOrCancelSignal &)>> handlers = {
			{DevelopJourneyStep::INITIAL_STEP, FsmSignalType::EnterState, &DevelopJourney::HandleInitialEnterSignal, nullptr},

			{DevelopJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::EnterState, &DevelopJourney::HandleResourceEnterSignal, nullptr},
			{DevelopJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::Custom, nullptr, &DevelopJourney::HandleResourceMouseClickSignal},
			{DevelopJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::ExitState, &DevelopJourney::HandleResourceExitSignal, nullptr},

			{DevelopJourneyStep::EXPECT_CONFIRMATION, FsmSignalType::EnterState, &DevelopJourney::HandleConfirmationEnterSignal, nullptr},
	};
	return handle_signal_using_handler_map(handlers, *this, s);
}

std::optional<DevelopJourneyStep> DevelopJourney::HandleInitialEnterSignal() {
	if (1 < PlayerIndustryTileCount(M2_PLAYER.character())) {
		if (auto selection = ask_for_confirmation_with_cancellation("Develop two industries at once? (Requires 2 Irons instead of 1)", "Yes", "No"); not selection) {
			M2_DEFER(m2g::Proxy::main_journey_deleter);
			return std::nullopt;
		} else {
			_develop_double_tiles = *selection;
		}
	}

	// Card selection
	if (auto selected_card = ask_for_card_selection()) {
		_selected_card = *selected_card;
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}

	// Tile selection
	if (auto selected_tile = ask_for_tile_selection()) {
		_selected_tile_1 = *selected_tile;
		if (_develop_double_tiles) {
			if (auto selected_tile_2 = ask_for_tile_selection(*selected_tile)) {
				_selected_tile_2 = *selected_tile_2;
			} else {
				M2_DEFER(m2g::Proxy::main_journey_deleter);
				return std::nullopt;
			}
		}
	} else {
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}

	return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
}

std::optional<DevelopJourneyStep> DevelopJourney::HandleResourceEnterSignal() {
	// Check if there's an unspecified iron source
	if (_iron_source_1 == 0 || (_develop_double_tiles && _iron_source_2 == 0)) {
		if (auto iron_industries = find_iron_industries_with_iron(); iron_industries.empty()) {
			// If no iron has left on the map, all the remaining iron must come from the market
			auto remaining_unspecified_iron_count = (_iron_source_1 == 0 && (_develop_double_tiles && _iron_source_2 == 0)) ? 2 : 1;
			// Calculate the cost of buying iron
			auto cost_of_buying = M2G_PROXY.market_iron_cost(m2::I(remaining_unspecified_iron_count));
			LOG_DEBUG("Asking player if they want to buy iron from the market...");
			if (ask_for_confirmation("Buy " + m2::ToString(remaining_unspecified_iron_count) + " iron from market for £" + m2::ToString(cost_of_buying) + "?", "", "Yes", "No")) {
				LOG_DEBUG("Player agreed");
				// Specify resource sources
				if (_iron_source_1 == 0) {
					_iron_source_1 = GLOUCESTER_1;
				}
				if (_develop_double_tiles && _iron_source_2 == 0) {
					_iron_source_2 = GLOUCESTER_1;
				}
				// Re-enter resource selection
				return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
			} else {
				LOG_INFO("Player declined, cancelling Develop action...");
				M2_DEFER(m2g::Proxy::main_journey_deleter);
			}
		} else if (iron_industries.size() == 1) {
			// Only one viable iron industry with iron is in the vicinity, confirm with the player.
			// Get a game drawing centered at the industry location
			auto background = M2_GAME.DrawGameToTexture(std::get<m2::VecF>(M2G_PROXY.industry_positions[*iron_industries.begin()]));
			LOG_DEBUG("Asking player if they want to buy iron from the closest industry...");
			if (ask_for_confirmation_bottom("Buy iron from shown industry for free?", "Yes", "No", std::move(background))) {
				LOG_DEBUG("Player agreed");
				// Reserve resource
				auto* factory = FindFactoryAtLocation(*iron_industries.begin());
				factory->character().remove_resource(IRON_CUBE_COUNT, 1.0f);
				((_iron_source_1 == 0) ? _reserved_source_1 : _reserved_source_2) = factory;
				// Specify resource source
				((_iron_source_1 == 0) ? _iron_source_1 : _iron_source_2) = *iron_industries.begin();
				// Re-enter resource selection
				return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
			} else {
				LOG_INFO("Player declined, cancelling Develop action...");
				M2_DEFER(m2g::Proxy::main_journey_deleter);
			}
		} else {
			sub_journey.emplace(iron_industries, "Pick an iron source using right mouse button...");
		}
		return std::nullopt;
	} else {
		return DevelopJourneyStep::EXPECT_CONFIRMATION;
	}
}

std::optional<DevelopJourneyStep> DevelopJourney::HandleResourceMouseClickSignal(const POIOrCancelSignal& s) {
	if (s.poi()) {
		auto industry_location = *s.poi();

		// Check if location has a built factory
		if (auto *factory = FindFactoryAtLocation(industry_location)) {
			// Check if the location is one of the dimming exceptions
			if (M2_LEVEL.dimming_exceptions()->contains(factory->id())) {
				// Deduct resource
				factory->character().remove_resource(IRON_CUBE_COUNT, 1.0f);
				// Save source
				if (_iron_source_1 == 0) {
					_iron_source_1 = industry_location;
					_reserved_source_1 = factory;
					if (_develop_double_tiles) {
						return std::nullopt;
					} else {
						return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
					}
				} else if (_develop_double_tiles && _iron_source_2 == 0) {
					_iron_source_2 = industry_location;
					_reserved_source_2 = factory;
					return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					throw M2_ERROR("Invalid state");
				}
			}
		}
		return std::nullopt;
	} else {
		LOG_INFO("Cancelling Develop action...");
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}
}

std::optional<DevelopJourneyStep> DevelopJourney::HandleResourceExitSignal() {
	sub_journey.reset();
	return std::nullopt;
}

std::optional<DevelopJourneyStep> DevelopJourney::HandleConfirmationEnterSignal() {
	LOG_INFO("Asking for confirmation...");

	auto confirmation = _develop_double_tiles
		? ask_for_confirmation("Develop " + ItemType_Name(_selected_tile_1), "and " + ItemType_Name(_selected_tile_2) + "?", "Yes", "No")
		: ask_for_confirmation("Develop " + ItemType_Name(_selected_tile_1) + "?", "", "Yes", "No");
	if (confirmation) {
		LOG_INFO("Develop action confirmed");

		m2g::pb::ClientCommand cc;
		cc.mutable_develop_action()->set_card(_selected_card);
		cc.mutable_develop_action()->set_industry_tile_1(_selected_tile_1);
		cc.mutable_develop_action()->set_industry_tile_2(_selected_tile_2);
		cc.mutable_develop_action()->set_iron_sources_1(_iron_source_1);
		cc.mutable_develop_action()->set_iron_sources_2(_iron_source_2);
		M2_GAME.QueueClientCommand(cc);
	} else {
		LOG_INFO("Cancelling Develop action...");
	}
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

bool CanPlayerDevelop(m2::Character& player, const m2g::pb::ClientCommand_DevelopAction& develop_action) {
	// Check if the prerequisites are met
	if (auto prerequisite = CanPlayerAttemptToDevelop(player); not prerequisite) {
		LOG_INFO("player does not meet develop prerequisites", prerequisite.error());
		return false;
	}

	// Check if the player holds the selected card
	if (not is_card(develop_action.card())) {
		LOG_WARN("Selected card is not a card");
		return false;
	}
	if (player.find_items(develop_action.card()) == player.end_items()) {
		LOG_WARN("Player does not have the selected card");
		return false;
	}

	// Check if the player holds the selected tiles
	if (not is_industry_tile(develop_action.industry_tile_1())
		|| (develop_action.industry_tile_2() && not is_industry_tile(develop_action.industry_tile_2()))) {
		LOG_WARN("Selected industry tile is not an industry tile");
		return false;
	}
	if (player.find_items(develop_action.industry_tile_1()) == player.end_items()
		|| (develop_action.industry_tile_2() && player.find_items(develop_action.industry_tile_2()) == player.end_items())) {
		LOG_WARN("Player does not have the selected tile");
		return false;
	}

	// Check if the tiles are the next tiles
	const auto& selected_industry_tile_1 = M2_GAME.GetNamedItem(develop_action.industry_tile_1());
	auto next_industry_tile_1 = PlayerNextIndustryTileOfCategory(player, selected_industry_tile_1.category());
	if (not next_industry_tile_1 || *next_industry_tile_1 != develop_action.industry_tile_1()) {
		LOG_INFO("Player cannot develop the selected tile");
		return false;
	}
	if (develop_action.industry_tile_2()) {
		// Reserve the first tile
		player.remove_item(player.find_items(develop_action.industry_tile_1()));
		// Check the tile
		const auto& selected_industry_tile_2 = M2_GAME.GetNamedItem(develop_action.industry_tile_2());
		auto next_industry_tile_2 = PlayerNextIndustryTileOfCategory(player, selected_industry_tile_2.category());
		auto success = true;
		if (not next_industry_tile_2 || *next_industry_tile_2 != develop_action.industry_tile_2()) {
			LOG_INFO("Player cannot develop the selected tile");
			success = false;
		}
		// Give the tile back
		player.add_named_item_no_benefits(M2_GAME.GetNamedItem(develop_action.industry_tile_1()));
		if (not success) {
			return false;
		}
	}

	// Check if the tile can be developed
	if (m2::is_equal(selected_industry_tile_1.get_attribute(DEVELOPMENT_BAN), 1.0f, 0.001f)) {
		LOG_INFO("Selected tile cannot be developed");
		return false;
	}
	if (develop_action.industry_tile_2()) {
		const auto& selected_industry_tile_2 = M2_GAME.GetNamedItem(develop_action.industry_tile_2());
		if (m2::is_equal(selected_industry_tile_2.get_attribute(DEVELOPMENT_BAN), 1.0f, 0.001f)) {
			LOG_INFO("Selected tile cannot be developed");
			return false;
		}
	}

	// Check if the player can use the selected resources
	// Gather reserved resources so that they can be given back
	m2::Object* reserved_resource{};
	if (is_industry_location(develop_action.iron_sources_1())) {
		// If iron source is an industry, find_iron_industries_with_iron must return it
		auto iron_industries = find_iron_industries_with_iron();
		if (iron_industries.find(develop_action.iron_sources_1()) == iron_industries.end()) {
			LOG_WARN("Player provided an iron source from an industry that does not contain an iron");
			return false;
		} else {
			// Reserve resource
			auto* factory = FindFactoryAtLocation(develop_action.iron_sources_1());
			factory->character().remove_resource(IRON_CUBE_COUNT, 1.0f);
			reserved_resource = factory;
		}
	} else if (is_merchant_location(develop_action.iron_sources_1())) {
		// Check that find_iron_industries_with_iron returns empty
		if (find_iron_industries_with_iron().empty()) {
			// Buying from market
		} else {
			LOG_WARN("Player provided a merchant as iron source, but an iron cannot be bought from the market");
			return false;
		}
	} else {
		LOG_WARN("Player provided unknown source for iron");
		return false;
	}
	auto resource_sources_are_valid = true;
	if (develop_action.industry_tile_2()) {
		if (is_industry_location(develop_action.iron_sources_2())) {
			// If iron source is an industry, find_iron_industries_with_iron must return it
			auto iron_industries = find_iron_industries_with_iron();
			if (iron_industries.find(develop_action.iron_sources_2()) == iron_industries.end()) {
				LOG_WARN("Player provided an iron source from an industry that does not contain an iron");
				resource_sources_are_valid = false;
			} else {
				// Buying from industry
			}
		} else if (is_merchant_location(develop_action.iron_sources_2())) {
			// Check that find_iron_industries_with_iron returns empty
			if (find_iron_industries_with_iron().empty()) {
				// Buying from market
			} else {
				LOG_WARN("Player provided a merchant as iron source, but an iron cannot be bought from the market");
				resource_sources_are_valid = false;
			}
		} else {
			LOG_WARN("Player provided unknown source for iron");
			resource_sources_are_valid = false;
		}
	}
	if (reserved_resource) {
		reserved_resource->character().add_resource(IRON_CUBE_COUNT, 1.0f);
	}
	// Check if exploration finished with a success
	if (not resource_sources_are_valid) {
		LOG_WARN("Some or all selected resources are unreachable");
		return false;
	}

	// Check if the player has enough money to buy the resources
	auto iron_from_market = is_merchant_location(develop_action.iron_sources_1()) ? 1 : 0;
	iron_from_market += develop_action.industry_tile_2() && is_merchant_location(develop_action.iron_sources_2()) ? 1 : 0;
	if (m2::iround(player.get_resource(MONEY)) < M2G_PROXY.market_iron_cost(iron_from_market)) {
		LOG_INFO("Player does not have enough money");
		return false;
	}

	return true;
}

std::pair<Card,int> ExecuteDevelopAction(m2::Character& player, const m2g::pb::ClientCommand_DevelopAction& develop_action) {
	// Assume validation is done

	// Calculate the cost of buying the resources
	auto iron_from_market = is_merchant_location(develop_action.iron_sources_1()) ? 1 : 0;
	iron_from_market += develop_action.industry_tile_2() && is_merchant_location(develop_action.iron_sources_2()) ? 1 : 0;
	auto cost = M2G_PROXY.market_iron_cost(iron_from_market);

	// Take resources
	if (is_industry_location(develop_action.iron_sources_1())) {
		auto* factory = FindFactoryAtLocation(develop_action.iron_sources_1());
		factory->character().remove_resource(IRON_CUBE_COUNT, 1.0f);
	} else if (is_merchant_location(develop_action.iron_sources_1())) {
		M2G_PROXY.buy_iron_from_market();
	}
	if (develop_action.industry_tile_2()) {
		if (is_industry_location(develop_action.iron_sources_2())) {
			auto* factory = FindFactoryAtLocation(develop_action.iron_sources_2());
			factory->character().remove_resource(IRON_CUBE_COUNT, 1.0f);
		} else if (is_merchant_location(develop_action.iron_sources_2())) {
			M2G_PROXY.buy_iron_from_market();
		}
	}

	// Take tile from player
	player.remove_item(player.find_items(develop_action.industry_tile_1()));
	if (develop_action.industry_tile_2()) {
		player.remove_item(player.find_items(develop_action.industry_tile_2()));
	}

	FlipExhaustedFactories();

	return std::make_pair(develop_action.card(), cost);
}
