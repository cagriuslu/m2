#include <cuzn/journeys/DevelopJourney.h>
#include <m2/Log.h>
#include "m2/Game.h"
#include "cuzn/ui/Detail.h"
#include <cuzn/ConsumingIron.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/GameStateTracker.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

m2::void_expected can_player_attempt_to_develop(m2::Character& player) {
	if (player_card_count(player) < 1) {
		return m2::make_unexpected("Develop action requires a card");
	}

	if (player_tile_count(player) < 1) {
		return m2::make_unexpected("Develop action requires an industry tile");
	}

	return {};
}

DevelopJourney::DevelopJourney() : FsmBase() {
	DEBUG_FN();
	init(DevelopJourneyStep::INITIAL_STEP);
}

DevelopJourney::~DevelopJourney() {
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

std::optional<DevelopJourneyStep> DevelopJourney::handle_signal(const PositionOrCancelSignal& s) {
	switch (state()) {
		case DevelopJourneyStep::INITIAL_STEP:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_initial_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2_ERROR("Unexpected signal");
			}
			break;
		case DevelopJourneyStep::EXPECT_RESOURCE_SOURCE:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_resource_enter_signal();
				case FsmSignalType::Custom: {
					if (auto world_position = s.world_position(); world_position) {
						return handle_resource_mouse_click_signal(*world_position);
					} else if (s.cancel()) {
						return handle_resource_cancel_signal();
					}
					throw M2_ERROR("Unexpected signal");
				}
				case FsmSignalType::ExitState: return handle_resource_exit_signal();
				default: throw M2_ERROR("Unexpected signal");
			}
		case DevelopJourneyStep::EXPECT_CONFIRMATION:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_confirmation_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2_ERROR("Unexpected signal");
			}
	}
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_initial_enter_signal() {
	if (1 < player_tile_count(M2_PLAYER.character())) {
		_develop_double_tiles = ask_for_confirmation("Develop two industries?", "", "Yes", "No");
	}

	// Card selection
	if (auto selected_card = ask_for_card_selection()) {
		_selected_card = *selected_card;
	} else {
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}

	// Tile selection
	if (auto selected_tile = ask_for_tile_selection()) {
		_selected_tile_1 = *selected_tile;
		if (_develop_double_tiles) {
			if (auto selected_tile_2 = ask_for_tile_selection(*selected_tile)) {
				_selected_tile_2 = *selected_tile_2;
			} else {
				M2_DEFER(m2g::Proxy::user_journey_deleter);
				return std::nullopt;
			}
		}
	} else {
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}

	return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_resource_enter_signal() {
	// Check if there's an unspecified iron source
	if (_iron_source_1 == 0 || (_develop_double_tiles && _iron_source_2 == 0)) {
		if (auto iron_industries = find_iron_industries_with_iron(); iron_industries.empty()) {
			// If no iron has left on the map, all the remaining iron must come from the market
			auto remaining_unspecified_iron_count = (_iron_source_1 == 0 && (_develop_double_tiles && _iron_source_2 == 0)) ? 2 : 1;
			// Calculate the cost of buying iron
			auto cost_of_buying = market_iron_cost(m2::I(remaining_unspecified_iron_count));
			LOG_DEBUG("Asking player if they want to buy iron from the market...");
			if (ask_for_confirmation("Buy " + std::to_string(remaining_unspecified_iron_count) + " iron from market for £" + std::to_string(cost_of_buying) + "?", "", "Yes", "No")) {
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
				M2_DEFER(m2g::Proxy::user_journey_deleter);
			}
		} else if (iron_industries.size() == 1) {
			// Only one viable iron industry with iron is in the vicinity, confirm with the player.
			// Get a game drawing centered at the industry location
			auto background = M2_GAME.draw_game_to_texture(std::get<m2::VecF>(M2G_PROXY.industry_positions[iron_industries[0]]));
			LOG_DEBUG("Asking player if they want to buy iron from the closest industry...");
			if (ask_for_confirmation_bottom("Buy iron from shown industry for free?", "Yes", "No", std::move(background))) {
				LOG_DEBUG("Player agreed");
				// Reserve resource
				auto* factory = find_factory_at_location(iron_industries[0]);
				factory->character().remove_resource(IRON_CUBE_COUNT, 1.0f);
				((_iron_source_1 == 0) ? _reserved_source_1 : _reserved_source_2) = factory;
				// Specify resource source
				((_iron_source_1 == 0) ? _iron_source_1 : _iron_source_2) = iron_industries[0];
				// Re-enter resource selection
				return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
			} else {
				LOG_INFO("Player declined, cancelling Develop action...");
				M2_DEFER(m2g::Proxy::user_journey_deleter);
			}
		} else {
			// Look-up ObjectIDs of the applicable industries
			std::set<ObjectId> iron_industry_object_ids;
			std::transform(iron_industries.begin(), iron_industries.end(),
				std::inserter(iron_industry_object_ids, iron_industry_object_ids.begin()),
				[](IndustryLocation loc) { return find_factory_at_location(loc)->id(); });
			// Enable dimming except the iron industries
			M2_GAME.enable_dimming_with_exceptions(iron_industry_object_ids);
			LOG_DEBUG("Asking player to pick an iron source...");

			M2_LEVEL.disable_hud();
			M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);
			M2_LEVEL.display_message("Pick an iron source");
		}
		return std::nullopt;
	} else {
		return DevelopJourneyStep::EXPECT_CONFIRMATION;
	}
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_resource_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);

	if (auto industry_loc = industry_location_on_position(world_position)) {
		LOG_DEBUG("Industry location", m2g::pb::SpriteType_Name(*industry_loc));
		// Check if location has a built factory
		if (auto* factory = find_factory_at_location(*industry_loc)) {
			// Check if the location is one of the dimming exceptions
			if (M2_GAME.dimming_exceptions()->contains(factory->id())) {
				// Deduct resource
				factory->character().remove_resource(IRON_CUBE_COUNT, 1.0f);
				// Save source
				if (_iron_source_1 == 0) {
					_iron_source_1 = *industry_loc;
					_reserved_source_1 = factory;
					if (_develop_double_tiles) {
						return std::nullopt;
					} else {
						return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
					}
				} else if (_develop_double_tiles && _iron_source_2 == 0) {
					_iron_source_2 = *industry_loc;
					_reserved_source_2 = factory;
					return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					throw M2_ERROR("Invalid state");
				}
			}
		}
	}
	return std::nullopt;
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_resource_cancel_signal() {
	LOG_INFO("Cancelling Develop action...");
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_resource_exit_signal() {
	M2_LEVEL.enable_hud();
	M2_LEVEL.remove_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
	// Disable dimming in case it was enabled
	M2_GAME.disable_dimming_with_exceptions();
	return std::nullopt;
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_confirmation_enter_signal() {
	LOG_INFO("Asking for confirmation...");

	auto confirmation = _develop_double_tiles
		? ask_for_confirmation("Develop " + ItemType_Name(_selected_tile_1), "and " + ItemType_Name(_selected_tile_1) + "?", "Yes", "No")
		: ask_for_confirmation("Develop " + ItemType_Name(_selected_tile_1) + "?", "", "Yes", "No");
	if (confirmation) {
		LOG_INFO("Develop action confirmed");

		m2g::pb::ClientCommand cc;
		cc.mutable_develop_action()->set_card(_selected_card);
		cc.mutable_develop_action()->set_industry_tile_1(_selected_tile_1);
		cc.mutable_develop_action()->set_industry_tile_2(_selected_tile_2);
		cc.mutable_develop_action()->set_iron_sources_1(_iron_source_1);
		cc.mutable_develop_action()->set_iron_sources_2(_iron_source_2);
		M2_GAME.client_thread().queue_client_command(cc);
	} else {
		LOG_INFO("Cancelling Develop action...");
	}
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}
