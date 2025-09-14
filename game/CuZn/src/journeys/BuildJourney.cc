#include <cuzn/journeys/BuildJourney.h>
#include <cuzn/Detail.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <cuzn/ConsumingCoal.h>
#include <cuzn/ui/Selection.h>
#include <cuzn/ui/IndustrySelection.h>
#include <cuzn/ConsumingIron.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2g/Proxy.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>
#include <cuzn/detail/Network.h>

using namespace m2;
using namespace m2g;
using namespace m2g::pb;

namespace {
	std::set<IndustryLocation> buildable_industry_locations_in_network_with_card(const Character& player, Card card) {
		if (not is_card(card)) {
			throw M2_ERROR("Item is not a card");
		}
		if (player.FindItems(card) == player.EndItems()) {
			throw M2_ERROR("Player does not own the given card");
		}

		// Gather industry locations in player's network
		std::set<IndustryLocation> industry_locations_in_network;
		if (auto cities_in_network = PlayerCitiesInNetwork(player); cities_in_network.empty() || card == WILD_LOCATION_CARD) {
			// If there are no locations in network, or the wild location card is selected, all locations are considered in-network
			industry_locations_in_network = all_industry_locations();
		} else {
			if (M2_GAME.named_items[card].Category() == ITEM_CATEGORY_CITY_CARD) {
				// If the card is a city card, add the city to list of cities
				cities_in_network.insert(card);
			}

			if (M2G_PROXY.is_canal_era()) {
				// If canal era, remove the cities where there is already an industry of player
				for (const auto& built_factory_location : PlayerBuiltFactoryLocations(player)) {
					cities_in_network.erase(city_of_location(built_factory_location));
					// For overbuilding, include the location of the already built industry
					industry_locations_in_network.insert(built_factory_location);
				}
			}

			for (const auto& city : cities_in_network) {
				// Insert locations in cities
				auto locs_in_city = industry_locations_in_city(city);
				industry_locations_in_network.insert(locs_in_city.begin(), locs_in_city.end());
			}
		}

		// Filter out already built locations
		for (auto it = industry_locations_in_network.begin(); it != industry_locations_in_network.end(); ) {
			if (FindFactoryAtLocation(*it)) {
				if (PlayerCanOverbuild(player, *it, card)) {
					++it;
				} else {
					it = industry_locations_in_network.erase(it);
				}
			} else {
				++it;
			}
		}

		// Filter by the type of the card
		if (card == WILD_LOCATION_CARD || card == WILD_INDUSTRY_CARD) {
			// No filtering
			return industry_locations_in_network;
		} else if (M2_GAME.named_items[card].Category() == ITEM_CATEGORY_INDUSTRY_CARD) {
			// Filter by industry
			auto filtered_locations = industry_locations_in_network | std::views::filter([card](IndustryLocation location) {
				auto industries = industries_on_location(location);
				return std::find(industries.begin(), industries.end(), card) != industries.end();
			});
			return {filtered_locations.begin(), filtered_locations.end()};
		} else if (M2_GAME.named_items[card].Category() == ITEM_CATEGORY_CITY_CARD) {
			// Filter by city
			auto filtered_locations = industry_locations_in_network | std::views::filter([card](IndustryLocation location) {
				return city_of_location(location) == card;
			});
			return {filtered_locations.begin(), filtered_locations.end()};
		} else {
			throw M2_ERROR("Invalid card category");
		}
	}

	std::vector<Industry> buildable_industries_with_card_on_location(ItemType selected_card, const SpriteType selected_location) {
		if (not is_card(selected_card)) {
			throw M2_ERROR("Item is not a card");
		}
		if (not is_industry_location(selected_location)) {
			throw M2_ERROR("Sprite is not an industry location");
		}

		const auto& selected_card_item = M2_GAME.GetNamedItem(selected_card);
		const auto& selected_sprite_sprite = std::get<Sprite>(M2_GAME.GetSpriteOrTextLabel(selected_location));
		// Lookup industries on the sprite
		std::vector<ItemType> selected_sprite_industries;
		std::ranges::copy_if(selected_sprite_sprite.NamedItems(), std::back_inserter(selected_sprite_industries), [](auto item_type) {
			return (M2_GAME.GetNamedItem(item_type).Category() == ITEM_CATEGORY_INDUSTRY_CARD);
		});
		if (selected_sprite_industries.empty()) {
			throw M2_ERROR("Selected sprite does not hold any industry cards");
		}

		// If overbuilding
		if (auto* factory = FindFactoryAtLocation(selected_location)) {
			// Only the type of the factory can be built
			return {ToIndustryOfFactoryCharacter(factory->GetCharacter())};
		}

		// If the card is wild card
		if (selected_card_item.Category() == ITEM_CATEGORY_WILD_CARD) {
			// Any industry in the selected location can be built
			return selected_sprite_industries;
		} else if (selected_card_item.Category() == ITEM_CATEGORY_INDUSTRY_CARD) {
			// Check if the selected industry exists in the sprite's industries
			auto industry_card_it = std::find(selected_sprite_industries.begin(), selected_sprite_industries.end(), selected_card);
			if (industry_card_it == selected_sprite_industries.end()) {
				return {}; // No buildable industries
			}
			return {selected_card}; // Only the selected industry card is buildable
		} else { // ITEM_CATEGORY_CITY_CARD
			// Look up the location of the sprite
			auto location_card_it = std::ranges::find_if(selected_sprite_sprite.NamedItems(), [](auto item_type) {
				return (M2_GAME.GetNamedItem(item_type).Category() == ITEM_CATEGORY_CITY_CARD);
			});
			if (location_card_it == selected_sprite_sprite.NamedItems().end()) {
				throw M2_ERROR("Selected sprite does not hold a location card");
			}
			ItemType selected_sprite_location = *location_card_it;
			// Check if the card belongs to this location
			if (selected_card == selected_sprite_location) {
				// Any industry in the selected location can be built
				return selected_sprite_industries;
			}
			return {}; // No buildable industries
		}
	}

	bool is_next_tile_higher_level_than_built_tile(const Character& factory_character, const IndustryTile next_industry_tile) {
		auto built_industry_tile_type = ToIndustryTileOfFactoryCharacter(factory_character);
		const auto& built_industry_tile_item = M2_GAME.GetNamedItem(built_industry_tile_type);
		const auto& next_industry_tile_item = M2_GAME.GetNamedItem(next_industry_tile);
		return IsLess(
				built_industry_tile_item.GetAttribute(TILE_LEVEL),
				next_industry_tile_item.GetAttribute(TILE_LEVEL),
				0.001f);
	}
}

void_expected CanPlayerAttemptToBuild(const Character& player) {
	if (PlayerCardCount(player) < 1) {
		return make_unexpected("Build action requires a card");
	}
	if (PlayerIndustryTileCount(player) < 1) {
		return make_unexpected("Build action requires an industry tile");
	}
	return {};
}

BuildJourney::BuildJourney() : FsmBase<BuildJourneyStep, POIOrCancelSignal>() {
	DEBUG_FN();
	init(BuildJourneyStep::INITIAL_STEP);
}

BuildJourney::~BuildJourney() {
	deinit();
	// Return the reserved resources
	for (auto [factory, resource_type] : _reserved_resources) {
		factory->GetCharacter().AddResource(resource_type, 1.0f);
	}
	_reserved_resources.clear();
}

std::optional<BuildJourneyStep> BuildJourney::HandleSignal(const POIOrCancelSignal& s) {
	static const std::initializer_list<std::tuple<
			BuildJourneyStep,
			FsmSignalType,
			std::optional<BuildJourneyStep>(BuildJourney::*)(),
			std::optional<BuildJourneyStep>(BuildJourney::*)(const POIOrCancelSignal&)>>& handlers = {
		{BuildJourneyStep::INITIAL_STEP, FsmSignalType::EnterState, &BuildJourney::HandleInitialEnterSignal, nullptr},

		{BuildJourneyStep::EXPECT_LOCATION, FsmSignalType::EnterState, &BuildJourney::HandleLocationEnterSignal, nullptr},
		{BuildJourneyStep::EXPECT_LOCATION, FsmSignalType::Custom, nullptr, &BuildJourney::HandleLocationMouseClickSignal},
		{BuildJourneyStep::EXPECT_LOCATION, FsmSignalType::ExitState, &BuildJourney::HandleLocationExitSignal, nullptr},

		{BuildJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::EnterState, &BuildJourney::HandleResourceEnterSignal, nullptr},
		{BuildJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::Custom, nullptr, &BuildJourney::HandleResourceMouseClickSignal},
		{BuildJourneyStep::EXPECT_RESOURCE_SOURCE, FsmSignalType::ExitState, &BuildJourney::HandleResourceExitSignal, nullptr},

		{BuildJourneyStep::EXPECT_CONFIRMATION, FsmSignalType::EnterState, &BuildJourney::HandleConfirmationEnterSignal, nullptr},
	};
	return handle_signal_using_handler_map(handlers, *this, s);
}

std::optional<BuildJourneyStep> BuildJourney::HandleInitialEnterSignal() {
	LOG_INFO("Entering build journey");
	if (const auto selected_card = ask_for_card_selection(); selected_card) {
		_selected_card = *selected_card;
		return BuildJourneyStep::EXPECT_LOCATION;
	}
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}
std::optional<BuildJourneyStep> BuildJourney::HandleLocationEnterSignal() {
	if (const auto buildableLocations = buildable_industry_locations_in_network_with_card(M2_PLAYER.GetCharacter(), _selected_card); buildableLocations.empty()) {
		M2_LEVEL.ShowMessage("No buildable locations in network is found", 8.0f);
	} else {
		sub_journey.emplace(buildableLocations, "Pick a location using right mouse button...");
	}
	return std::nullopt;
}
std::optional<BuildJourneyStep> BuildJourney::HandleLocationMouseClickSignal(const POIOrCancelSignal& s) {
	if (s.poi()) {
		const auto selected_location = *s.poi();
		// Check if there's a need to make an industry selection based on the card and the sprite
		if (const auto buildable_inds = buildable_industries_with_card_on_location(_selected_card, selected_location); buildable_inds.empty()) {
			throw M2_ERROR("Implementation error, point-of-interest does not have a buildable location");
		} else if (buildable_inds.size() == 2) {
			if (const auto selected_industry = ask_for_industry_selection(buildable_inds[0], buildable_inds[1]); selected_industry) {
				_selected_industry = *selected_industry;
			} else {
				M2_DEFER(m2g::Proxy::main_journey_deleter);
				return std::nullopt;
			}
		} else if (buildable_inds.size() == 1) {
			_selected_industry = buildable_inds[0];
		} else {
			throw M2_ERROR("Implementation error, more than 2 selectable industries in one location");
		}
		_selected_location = selected_location;

		// Check if the player has a factory to build
		auto tile_type = PlayerNextIndustryTileOfCategory(M2_PLAYER.GetCharacter(), industry_tile_category_of_industry(_selected_industry));
		if (not tile_type) {
			M2_LEVEL.ShowMessage("No industry tile of appropriate type is found", 8.0f);
			M2_DEFER(m2g::Proxy::main_journey_deleter);
			return std::nullopt;
		}
		// If overbuilding
		if (auto* factory = FindFactoryAtLocation(selected_location)) {
			// The next tile must be higher level than the built industry
			if (not is_next_tile_higher_level_than_built_tile(factory->GetCharacter(), *tile_type)) {
				M2_LEVEL.ShowMessage("Overbuilding requires a higher level tile", 8.0f);
				M2_DEFER(m2g::Proxy::main_journey_deleter);
				return std::nullopt;
			}
		}
		_industry_tile = *tile_type;

		// Create empty entries in resource_sources for every required resource
		_resource_sources.insert(_resource_sources.end(),
				RoundI(M2_GAME.GetNamedItem(*tile_type).GetAttribute(COAL_COST)),
				std::make_pair(COAL_CUBE_COUNT, NO_SPRITE));
		_resource_sources.insert(_resource_sources.end(),
				RoundI(M2_GAME.GetNamedItem(*tile_type).GetAttribute(IRON_COST)),
				std::make_pair(IRON_CUBE_COUNT, NO_SPRITE));
		return _resource_sources.empty() ? BuildJourneyStep::EXPECT_CONFIRMATION : BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
	} else {
		LOG_INFO("Cancelling Build action...");
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}
}
std::optional<BuildJourneyStep> BuildJourney::HandleLocationExitSignal() {
	sub_journey.reset();
	return std::nullopt;
}
std::optional<BuildJourneyStep> BuildJourney::HandleResourceEnterSignal() {
	// Check if there's an unspecified resource left
	if (auto unspecified_resource = GetNextUnspecifiedResource(); unspecified_resource != _resource_sources.end()) {
		if (unspecified_resource->first == COAL_CUBE_COUNT) {
			auto selected_city = city_of_location(_selected_location);
			if (auto closest_mines_with_coal = find_closest_connected_coal_mines_with_coal(selected_city); closest_mines_with_coal.empty()) {
				// No reachable coal mines with coal, check the coal market
				if (auto coal_market_city = find_connected_coal_market(selected_city)) {
					// If no reachable coal has left on the map, all the remaining coal must come from the market
					auto remaining_unspecified_coal_count = std::count(_resource_sources.begin(), _resource_sources.end(),
						std::make_pair(COAL_CUBE_COUNT, NO_SPRITE));
					// Calculate the cost of buying coal
					auto cost_of_buying = M2G_PROXY.market_coal_cost(m2::I(remaining_unspecified_coal_count));
					// Merchant location
					auto merchant_location = merchant_locations_of_merchant_city(*coal_market_city)[0];
					// Get a game drawing centered at the merchant location
					auto background = M2_GAME.DrawGameToTexture(std::get<m2::VecF>(M2G_PROXY.merchant_positions[merchant_location]));
					LOG_DEBUG("Asking player if they want to buy coal from the market...");
					if (ask_for_confirmation_bottom("Buy " + m2::ToString(remaining_unspecified_coal_count) + " coal from market for £" + m2::ToString(cost_of_buying) + "?", "Yes", "No", std::move(background))) {
						LOG_DEBUG("Player agreed");
						// Specify resource sources
						std::ranges::replace(_resource_sources,
							std::make_pair(COAL_CUBE_COUNT, NO_SPRITE), std::make_pair(COAL_CUBE_COUNT, merchant_location));
						// Re-enter resource selection
						return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
					} else {
						LOG_INFO("Player declined, cancelling Build action...");
						M2_DEFER(m2g::Proxy::main_journey_deleter);
					}
				} else {
					M2_LEVEL.ShowMessage("Coal required but none available in network", 8.0f);
					M2_DEFER(m2g::Proxy::main_journey_deleter);
				}
			} else if (closest_mines_with_coal.size() == 1) {
				// Only one viable coal mine with coal is in the vicinity, confirm with the player.
				// Get a game drawing centered at the industry location
				auto background = M2_GAME.DrawGameToTexture(std::get<m2::VecF>(M2G_PROXY.industry_positions[*closest_mines_with_coal.begin()]));
				LOG_DEBUG("Asking player if they want to buy coal from the closest mine...");
				if (ask_for_confirmation_bottom("Buy coal from shown mine for free?", "Yes", "No", std::move(background))) {
					LOG_DEBUG("Player agreed");
					// Reserve resource
					auto* factory = FindFactoryAtLocation(*closest_mines_with_coal.begin());
					factory->GetCharacter().RemoveResource(COAL_CUBE_COUNT, 1.0f);
					_reserved_resources.emplace_back(factory, COAL_CUBE_COUNT);
					// Specify resource source
					unspecified_resource->second = *closest_mines_with_coal.begin();
					// Re-enter resource selection
					return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					LOG_INFO("Player declined, cancelling Build action...");
					M2_DEFER(m2g::Proxy::main_journey_deleter);
				}
			} else {
				// More than one coal mine are available at equal distance. Use POISelectionJourney to gather the
				// preference of the player.
				sub_journey.emplace(closest_mines_with_coal, "Pick a coal source using right mouse button...");
			}
		} else if (unspecified_resource->first == IRON_CUBE_COUNT) {
			if (auto iron_industries = find_iron_industries_with_iron(); iron_industries.empty()) {
				// If no iron has left on the map, all the remaining iron must come from the market
				auto remaining_unspecified_iron_count = std::count(_resource_sources.begin(), _resource_sources.end(),
					std::make_pair(IRON_CUBE_COUNT, NO_SPRITE));
				// Calculate the cost of buying iron
				auto cost_of_buying = M2G_PROXY.market_iron_cost(m2::I(remaining_unspecified_iron_count));
				LOG_DEBUG("Asking player if they want to buy iron from the market...");
				if (ask_for_confirmation("Buy " + m2::ToString(remaining_unspecified_iron_count) + " iron from market for £" + m2::ToString(cost_of_buying) + "?", "", "Yes", "No")) {
					LOG_DEBUG("Player agreed");
					// Specify resource sources
					std::replace(_resource_sources.begin(), _resource_sources.end(),
						std::make_pair(IRON_CUBE_COUNT, NO_SPRITE), std::make_pair(IRON_CUBE_COUNT, GLOUCESTER_1));
					// Re-enter resource selection
					return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					LOG_INFO("Player declined, cancelling Build action...");
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
					factory->GetCharacter().RemoveResource(IRON_CUBE_COUNT, 1.0f);
					_reserved_resources.emplace_back(factory, IRON_CUBE_COUNT);
					// Specify resource source
					unspecified_resource->second = *iron_industries.begin();
					// Re-enter resource selection
					return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					LOG_INFO("Player declined, cancelling Build action...");
					M2_DEFER(m2g::Proxy::main_journey_deleter);
				}
			} else {
				sub_journey.emplace(iron_industries, "Pick an iron source using right mouse button...");
			}
		} else {
			throw M2_ERROR("Unexpected resource in resource list");
		}
		return std::nullopt;
	} else {
		return BuildJourneyStep::EXPECT_CONFIRMATION;
	}
}
std::optional<BuildJourneyStep> BuildJourney::HandleResourceMouseClickSignal(const POIOrCancelSignal& s) {
	if (s.poi()) {
		auto industry_location = *s.poi();

		auto unspecified_resource = GetNextUnspecifiedResource();
		// Check if location has a built factory
		if (auto* factory = FindFactoryAtLocation(industry_location)) {
			// Check if the location is one of the dimming exceptions
			if (M2_LEVEL.GetDimmingExceptions()->contains(factory->GetId())) {
				// Reserve resource
				factory->GetCharacter().RemoveResource(unspecified_resource->first, 1.0f);
				// Specify resource source
				unspecified_resource->second = industry_location;
				// Reserve resource
				_reserved_resources.emplace_back(factory, unspecified_resource->first);
				// Re-enter resource selection
				return BuildJourneyStep::EXPECT_RESOURCE_SOURCE;
			}
		}
		return std::nullopt;
	} else {
		LOG_INFO("Cancelling Build action...");
		M2_DEFER(m2g::Proxy::main_journey_deleter);
		return std::nullopt;
	}
}
std::optional<BuildJourneyStep> BuildJourney::HandleResourceExitSignal() {
	sub_journey.reset();
	return std::nullopt;
}
std::optional<BuildJourneyStep> BuildJourney::HandleConfirmationEnterSignal() {
	LOG_INFO("Asking for confirmation...");
	auto card_name = M2_GAME.GetNamedItem(_selected_card).in_game_name();
	auto city_name = M2_GAME.GetNamedItem(city_of_location(_selected_location)).in_game_name();
	auto industry_name = M2_GAME.GetNamedItem(_selected_industry).in_game_name();
	if (ask_for_confirmation("Build " + industry_name + " in " + city_name, "using " + card_name + " card?", "OK", "Cancel")) {
		LOG_INFO("Build action confirmed");

		m2g::pb::TurnBasedClientCommand cc;
		cc.mutable_build_action()->set_card(_selected_card);
		cc.mutable_build_action()->set_industry_location(_selected_location);
		cc.mutable_build_action()->set_industry_tile(_industry_tile);
		for (const auto& resource_source : _resource_sources) {
			if (resource_source.first == COAL_CUBE_COUNT) {
				cc.mutable_build_action()->add_coal_sources(resource_source.second);
			} else if (resource_source.first == IRON_CUBE_COUNT) {
				cc.mutable_build_action()->add_iron_sources(resource_source.second);
			} else {
				throw M2_ERROR("Unexpected resource type");
			}
		}
		M2G_PROXY.SendClientCommandAndWaitForServerUpdate(cc);
	} else {
		LOG_INFO("Cancelling Build action...");
	}
	M2_DEFER(m2g::Proxy::main_journey_deleter);
	return std::nullopt;
}

decltype(BuildJourney::_resource_sources)::iterator BuildJourney::GetNextUnspecifiedResource() {
	return std::find_if(_resource_sources.begin(), _resource_sources.end(), [](const auto& r) {
		return r.second == NO_SPRITE;
	});
}

m2::void_expected CanPlayerBuild(m2::Character& player, const m2g::pb::TurnBasedClientCommand_BuildAction& build_action) {
	// Check if prerequisites are met
	if (auto prerequisite = CanPlayerAttemptToBuild(player); not prerequisite) {
		return make_unexpected(prerequisite.error());
	}

	// Check if the player holds the selected card
	if (not is_card(build_action.card())) {
		return make_unexpected("Selected card is not a card");
	}
	if (player.FindItems(build_action.card()) == player.EndItems()) {
		return make_unexpected("Player does not have the selected card");
	}

	// Check if the player has the selected tile
	if (not is_industry_tile(build_action.industry_tile())) {
		return make_unexpected("Selected industry tile is not an industry tile");
	}
	if (player.FindItems(build_action.industry_tile()) == player.EndItems()) {
		return make_unexpected("Player does not have the selected tile");
	}
	auto industry = industry_of_industry_tile(build_action.industry_tile());
	// Check if the tile is the next tile
	const auto& selected_industry_tile = M2_GAME.GetNamedItem(build_action.industry_tile());
	auto next_industry_tile = PlayerNextIndustryTileOfCategory(player, selected_industry_tile.Category());
	if (not next_industry_tile || *next_industry_tile != build_action.industry_tile()) {
		return make_unexpected("Player cannot use the selected tile");
	}
	// Check if the tile can be built in this era
	auto forbidden_era = selected_industry_tile.GetAttribute(m2g::pb::FORBIDDEN_ERA);
	if ((m2::IsEqual(forbidden_era, 1.0f, 0.001f) && M2G_PROXY.is_canal_era()) ||
		(m2::IsEqual(forbidden_era, 2.0f, 0.001f) && M2G_PROXY.is_railroad_era())) {
		return make_unexpected("Player selected an industry that cannot be built in this era");
	}

	// Check if the player can build the industry with the selected card
	if (not buildable_industry_locations_in_network_with_card(player, build_action.card()).contains(build_action.industry_location())) {
		return make_unexpected("Player selected an industry location that is not reachable or cannot be built with the selected card");
	}
	// If overbuilding, check if the built industry is selected
	if (auto* factory = FindFactoryAtLocation(build_action.industry_location())) {
		if (ToIndustryOfFactoryCharacter(factory->GetCharacter()) != industry_of_industry_tile(build_action.industry_tile())) {
			return make_unexpected("Player selected an industry type different from overbuilt industry");
		}
		if (not is_next_tile_higher_level_than_built_tile(factory->GetCharacter(), build_action.industry_tile())) {
			return make_unexpected("Player selected a tile with level not higher than overbuilt tile");
		}
	}
	auto city = city_of_location(build_action.industry_location());
	// If there's more than one industry on this location, check if there's another location in the city with only this industry.
	// The location with only one industry must be built before building the factory on a multi-industry location.
	if (auto industries = industries_on_location(build_action.industry_location()); 1 < industries.size()) {
		// Find all the locations in the city
		auto locations_in_city = industry_locations_in_city(city);
		// Remove the selected location
		locations_in_city.erase(std::remove(locations_in_city.begin(), locations_in_city.end(), build_action.industry_location()), locations_in_city.end());
		// Filter to locations that have only the selected industries
		auto other_locations_with_only_the_industry = locations_in_city | std::views::filter([industry](SpriteType loc) {
			const auto industries = industries_on_location(loc);
			return (industries.size() == 1 && industries[0] == industry);
		});
		auto is_all_other_locations_occupied = std::ranges::all_of(other_locations_with_only_the_industry, FindFactoryAtLocation);
		if (not is_all_other_locations_occupied) {
			return make_unexpected("Player cannot build on the selected location while the city has an empty location with only that industry");
		}
	}

	// Check if the player can use the selected resources
	// Gather required resources
	std::vector<std::pair<m2g::pb::ResourceType, Location>> resource_sources;
	// Create empty entries in resource_sources for every required resource
	resource_sources.insert(resource_sources.end(),
		RoundI(M2_GAME.GetNamedItem(build_action.industry_tile()).GetAttribute(COAL_COST)),
		std::make_pair(COAL_CUBE_COUNT, NO_SPRITE));
	resource_sources.insert(resource_sources.end(),
		RoundI(M2_GAME.GetNamedItem(build_action.industry_tile()).GetAttribute(IRON_COST)),
		std::make_pair(IRON_CUBE_COUNT, NO_SPRITE));
	// Gather reserved resources so that they can be given back
	std::vector<std::pair<m2::Object*, m2g::pb::ResourceType>> reserved_resources;
	bool resource_sources_are_valid = false;
	for (const auto& coal_source : build_action.coal_sources()) {
		auto location = static_cast<Location>(coal_source);
		auto next_unspecified_coal_resource = std::find(resource_sources.begin(), resource_sources.end(), std::make_pair(COAL_CUBE_COUNT, NO_SPRITE));
		if (is_industry_location(location)) {
			// If coal source is an industry, find_closest_connected_coal_mines_with_coal must return it
			auto closest_coal_mines = find_closest_connected_coal_mines_with_coal(city);
			if (std::find(closest_coal_mines.begin(), closest_coal_mines.end(), location) == closest_coal_mines.end()) {
				LOG_WARN("Player provided a coal source from a coal mine that's not one of the closest connected mines", coal_source);
				goto return_resources;
			} else {
				// Reserve resource
				auto* factory = FindFactoryAtLocation(location);
				factory->GetCharacter().RemoveResource(COAL_CUBE_COUNT, 1.0f);
				reserved_resources.emplace_back(factory, COAL_CUBE_COUNT);
				// Specify resource source
				next_unspecified_coal_resource->second = location;
			}
		} else if (is_merchant_location(location)) {
			// Check that find_closest_connected_coal_mines_with_coal returns empty, and there's a connection to some merchant
			if (find_closest_connected_coal_mines_with_coal(city).empty() && find_connected_coal_market(city)) {
				// Specify resource source
				next_unspecified_coal_resource->second = location;
			} else {
				LOG_WARN("Player provided a merchant as coal source, but a coal cannot be bought from the market");
				goto return_resources;
			}
		} else {
			LOG_WARN("Player provided unknown source for coal", coal_source);
			goto return_resources;
		}
	}
	for (const auto& iron_source : build_action.iron_sources()) {
		auto location = static_cast<Location>(iron_source);
		auto next_unspecified_iron_resource = std::find(resource_sources.begin(), resource_sources.end(), std::make_pair(IRON_CUBE_COUNT, NO_SPRITE));
		if (is_industry_location(location)) {
			// If iron source is an industry, find_iron_industries_with_iron must return it
			auto iron_industries = find_iron_industries_with_iron();
			if (std::find(iron_industries.begin(), iron_industries.end(), location) == iron_industries.end()) {
				LOG_WARN("Player provided an iron source from an industry that does not contain an iron", iron_source);
				goto return_resources;
			} else {
				// Reserve resource
				auto* factory = FindFactoryAtLocation(location);
				factory->GetCharacter().RemoveResource(IRON_CUBE_COUNT, 1.0f);
				reserved_resources.emplace_back(factory, IRON_CUBE_COUNT);
				// Specify resource source
				next_unspecified_iron_resource->second = location;
			}
		} else if (is_merchant_location(location)) {
			// Check that find_iron_industries_with_iron returns empty
			if (find_iron_industries_with_iron().empty()) {
				// Specify resource source
				next_unspecified_iron_resource->second = location;
			} else {
				LOG_WARN("Player provided a merchant as iron source, but an iron cannot be bought from the market");
				goto return_resources;
			}
		} else {
			LOG_WARN("Player provided unknown source for iron", iron_source);
			goto return_resources;
		}
	}
	resource_sources_are_valid = true;
return_resources:
	// Give back the reserved resources
	for (auto [factory, resource_type] : reserved_resources) {
		factory->GetCharacter().AddResource(resource_type, 1.0f);
	}
	// Check if exploration finished with a success
	if (not resource_sources_are_valid) {
		return make_unexpected("Some or all selected resources are unreachable");
	}

	// Check if all required resources are provided
	for (const auto& resource_source : resource_sources) {
		if (resource_source.second == NO_SPRITE) {
			return make_unexpected("Player provided no source for a required resource: " + m2::pb::enum_name(resource_source.first));
		}
	}

	auto coal_from_market = std::ranges::count_if(build_action.coal_sources(), [](const auto& coal_source) {
		return is_merchant_location(static_cast<Location>(coal_source));
	});
	auto iron_from_market = std::ranges::count_if(build_action.iron_sources(), [](const auto& iron_source) {
		return is_merchant_location(static_cast<Location>(iron_source));
	});
	// Check if the player has enough money
	if (RoundI(player.GetResource(MONEY)) < RoundI(M2_GAME.GetNamedItem(build_action.industry_tile()).GetAttribute(MONEY_COST)) +
		M2G_PROXY.market_coal_cost(I(coal_from_market)) + M2G_PROXY.market_iron_cost(I(iron_from_market))) {
		return make_unexpected("Player does not have enough money");
	}

	return {};
}

std::pair<Card,int> ExecuteBuildAction(m2::Character& player, const m2g::pb::TurnBasedClientCommand_BuildAction& build_action) {
	// Assume validation is done

	// Take tile from player
	const auto& tile_item = M2_GAME.GetNamedItem(build_action.industry_tile());
	auto tile_category = tile_item.Category();
	auto tile_type = PlayerNextIndustryTileOfCategory(player, tile_category);
	player.RemoveItem(player.FindItems(*tile_type));

	// Calculate the cost before building the industry
	auto coal_from_market = std::count_if(build_action.coal_sources().begin(), build_action.coal_sources().end(), [](const auto& coal_source) {
		return is_merchant_location(static_cast<Location>(coal_source));
	});
	auto iron_from_market = std::count_if(build_action.iron_sources().begin(), build_action.iron_sources().end(), [](const auto& iron_source) {
		return is_merchant_location(static_cast<Location>(iron_source));
	});
	auto cost = m2::RoundI(M2_GAME.GetNamedItem(build_action.industry_tile()).GetAttribute(MONEY_COST)) +
		M2G_PROXY.market_coal_cost(m2::I(coal_from_market)) + M2G_PROXY.market_iron_cost(m2::I(iron_from_market));

	// Take resources
	for (const auto& coal_source : build_action.coal_sources()) {
		auto location = static_cast<Location>(coal_source);
		if (is_industry_location(location)) {
			auto* factory = FindFactoryAtLocation(location);
			factory->GetCharacter().RemoveResource(COAL_CUBE_COUNT, 1.0f);
		} else if (is_merchant_location(location)) {
			M2G_PROXY.buy_coal_from_market();
		}
	}
	for (const auto& iron_source : build_action.iron_sources()) {
		auto location = static_cast<Location>(iron_source);
		if (is_industry_location(location)) {
			auto* factory = FindFactoryAtLocation(location);
			factory->GetCharacter().RemoveResource(IRON_CUBE_COUNT, 1.0f);
		} else if (is_merchant_location(location)) {
			M2G_PROXY.buy_iron_from_market();
		}
	}

	// If overbuilding
	if (auto* factory = FindFactoryAtLocation(build_action.industry_location())) {
		// Remove previous factory
		M2_DEFER(m2::CreateObjectDeleter(factory->GetId()));
	}
	// Create factory on the map
	auto it = m2::CreateObject(m2g::pb::FACTORY, player.OwnerId());
	auto city = city_of_location(build_action.industry_location());
	InitFactory(*it, position_of_industry_location(build_action.industry_location()), city, build_action.industry_tile());
	// Give resources to factory, sell to market at the same time
	if (tile_category == ITEM_CATEGORY_COAL_MINE_TILE) {
		// If there's a connection to coal market
		if (find_connected_coal_market(city)) {
			auto gained_resource_count = m2::RoundI(tile_item.GetAttribute(COAL_BONUS));
			auto [sell_count, revenue] = M2G_PROXY.market_coal_revenue(gained_resource_count);
			// Sell to market
			M2G_PROXY.sell_coal_to_market(sell_count);
			// Gain revenue
			player.AddResource(MONEY, m2::ToFloat(revenue));
			// Keep the rest
			it->GetCharacter().AddResource(COAL_CUBE_COUNT, m2::ToFloat(gained_resource_count - sell_count));
		} else {
			it->GetCharacter().AddResource(COAL_CUBE_COUNT, tile_item.GetAttribute(COAL_BONUS));
		}
	} else if (tile_category == ITEM_CATEGORY_IRON_WORKS_TILE) {
		auto gained_resource_count = m2::RoundI(tile_item.GetAttribute(IRON_BONUS));
		auto [sell_count, revenue] = M2G_PROXY.market_iron_revenue(gained_resource_count);
		// Sell to market
		M2G_PROXY.sell_iron_to_market(sell_count);
		// Gain revenue
		player.AddResource(MONEY, m2::ToFloat(revenue));
		// Keep the rest
		it->GetCharacter().AddResource(IRON_CUBE_COUNT, m2::ToFloat(gained_resource_count - sell_count));
	} else if (tile_category == ITEM_CATEGORY_BREWERY_TILE) {
		it->GetCharacter().AddResource(BEER_BARREL_COUNT, tile_item.GetAttribute(
			M2G_PROXY.is_canal_era() ? BEER_BONUS_FIRST_ERA : BEER_BONUS_SECOND_ERA));
	}

	FlipExhaustedFactories();

	return std::make_pair(build_action.card(), cost);
}
