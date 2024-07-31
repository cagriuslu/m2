#include <cuzn/journeys/SellJourney.h>
#include <m2/Log.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include "cuzn/object/Factory.h"
#include "cuzn/object/Merchant.h"
#include <cuzn/detail/Network.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

SellJourney::SellJourney() : m2::FsmBase<SellJourneyStep, PositionOrCancelSignal>() {
	DEBUG_FN();
	init(SellJourneyStep::INITIAL_STEP);
}

std::optional<SellJourneyStep> SellJourney::handle_signal(const PositionOrCancelSignal& s) {
	switch (state()) {
		case SellJourneyStep::INITIAL_STEP:
			switch (s.type()) {
				case m2::FsmSignalType::EnterState: return handle_initial_enter_signal();
				case m2::FsmSignalType::ExitState: return std::nullopt;
				default: throw M2_ERROR("Unexpected signal");
			}
		case SellJourneyStep::EXPECT_LOCATION:
			switch (s.type()) {
				case m2::FsmSignalType::EnterState: return handle_location_enter_signal();
				case m2::FsmSignalType::Custom: {
					if (auto world_position = s.world_position(); world_position) {
						return handle_location_mouse_click_signal(*world_position);
					} else if (s.cancel()) {
						return handle_location_cancel_signal();
					}
					throw M2_ERROR("Unexpected signal");
				}
				case m2::FsmSignalType::ExitState: return handle_location_exit_signal();
				default: throw M2_ERROR("Unexpected signal");
			}
		case SellJourneyStep::EXPECT_RESOURCE_SOURCE:
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
			return std::nullopt;
		case SellJourneyStep::EXPECT_CONFIRMATION:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_confirmation_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2_ERROR("Unexpected signal");
			}
	}
}

std::optional<SellJourneyStep> SellJourney::handle_initial_enter_signal() {
	if (auto selected_card = ask_for_card_selection(); selected_card) {
		_selected_card = *selected_card;
		return SellJourneyStep::EXPECT_LOCATION;
	} else {
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}
}

std::optional<SellJourneyStep> SellJourney::handle_location_enter_signal() {
	LOG_DEBUG("Expecting sell location...");
	M2_LEVEL.disable_hud();
	M2_LEVEL.display_message("Pick industry");
	M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_location_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);

	// Lookup industry location
	if (auto selected_loc = industry_location_on_position(world_position)) {
		LOG_INFO("Clicked on", m2g::pb::SpriteType_Name(*selected_loc));

		// Search for a factory on the selected location
		if (auto* factory = find_factory_at_location(*selected_loc)) {
			// Check the type of the industry
			auto industry = to_industry_of_factory_character(factory->character());
			if (industry == COTTON_MILL_CARD || industry == POTTERY_CARD || industry == MANUFACTURED_GOODS_CARD) {
				// Check if the owner of the factory is the player
				if (factory->parent_id() == M2_PLAYER.id()) {
					_selected_location = *selected_loc;

					// Lookup the IndustryTile of the factory
					auto industry_tile = to_industry_tile_of_factory_character(factory->character());
					auto required_beer_count = iround(M2_GAME.get_named_item(industry_tile).get_attribute(BEER_COST));
					// Create empty entries in _beer_sources for every required beer
					_beer_sources.insert(_beer_sources.end(), required_beer_count, NO_SPRITE);

					return _beer_sources.empty() ? SellJourneyStep::EXPECT_CONFIRMATION : SellJourneyStep::EXPECT_RESOURCE_SOURCE;
				} else {
					M2_LEVEL.display_message("Selected industry doesn't belong to you");
				}
			} else {
				M2_LEVEL.display_message("Selected industry cannot be sold");
			}
		} else {
			LOG_DEBUG("Selected position was not on a built industry");
		}
	} else {
		LOG_DEBUG("Selected position was not on an industry");
	}
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_location_cancel_signal() {
	LOG_INFO("Cancelling Sell action...");
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_location_exit_signal() {
	M2_LEVEL.enable_hud();
	M2_LEVEL.remove_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_resource_enter_signal() {
	// Check if there's an unspecified beer left
	if (auto unspecified_resource = std::find(_beer_sources.begin(), _beer_sources.end(), NO_SPRITE); unspecified_resource != _beer_sources.end()) {
		LOG_DEBUG("Expecting beer source...");
		M2_LEVEL.disable_hud();
		M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);

		auto index = std::distance(_beer_sources.begin(), unspecified_resource) + 1;
		M2_LEVEL.display_message(std::to_string(index) +  "/" + std::to_string(_beer_sources.size()) + ": Pick a beer source");
		return std::nullopt;
	} else {
		return SellJourneyStep::EXPECT_CONFIRMATION;
	}
}

std::optional<SellJourneyStep> SellJourney::handle_resource_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);

	auto unspecified_resource = std::find(_beer_sources.begin(), _beer_sources.end(), NO_SPRITE);
	// Lookup the IndustryLocation under the mouse
	if (auto industry_loc = industry_location_on_position(world_position)) {
		LOG_DEBUG("Industry location", m2g::pb::SpriteType_Name(*industry_loc));
		// Check if location has a built factory
		if (auto* factory = find_factory_at_location(*industry_loc)) {
			// Check if factory has the required resource
			if (m2::god_mode || m2::is_less_or_equal(1.0f, factory->character().get_resource(BEER_BARREL_COUNT), 0.001f)) {
				// If industry belongs to the player, connection isn't necessary. Otherwise, a connection is necessary.
				if (m2::god_mode || factory->parent_id() == M2_PLAYER.id() || is_industry_city_connected_to_location(city_of_location(_selected_location), *industry_loc)) {
					factory->character().remove_resource(BEER_BARREL_COUNT, 1.0f); // Deduct resource
					*unspecified_resource = *industry_loc; // Specify resource source
					_reserved_beers.emplace_back(factory); // Reserve resource
					return SellJourneyStep::EXPECT_RESOURCE_SOURCE; // Re-enter resource selection
				} else {
					LOG_DEBUG("There's no connection between selected industries");
				}
			} else {
				LOG_DEBUG("Industry doesn't have the required resource");
			}
		} else {
			LOG_DEBUG("Industry is not built");
		}
	} else if (auto merchant_loc = merchant_location_on_position(world_position)) {
		LOG_DEBUG("Merchant location", m2g::pb::SpriteType_Name(*merchant_loc));
		// Check if location has a merchant
		if (auto* merchant = find_merchant_at_location(*merchant_loc)) {
			// Check if merchant has the required resource
			if (m2::god_mode || m2::is_less_or_equal(1.0f, merchant->character().get_resource(BEER_BARREL_COUNT), 0.001f)) {
				// A connection is necessary
				if (m2::god_mode || is_industry_city_connected_to_location(city_of_location(_selected_location), *merchant_loc)) {
					merchant->character().remove_resource(BEER_BARREL_COUNT, 1.0f); // Deduct resource
					*unspecified_resource = *merchant_loc; // Specify resource source
					_reserved_beers.emplace_back(merchant); // Reserve resource
					return SellJourneyStep::EXPECT_RESOURCE_SOURCE; // Re-enter resource selection
				} else {
					LOG_DEBUG("There's no connection between the industry and the merchant");
				}
			} else {
				LOG_DEBUG("Merchant doesn't have the required resource");
			}
		} else {
			LOG_DEBUG("Merchant is not active");
		}
	}
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_resource_cancel_signal() {
	LOG_INFO("Cancelling Sell action...");
	// Return the reserved resources
	for (auto* object : _reserved_beers) {
		object->character().add_resource(BEER_BARREL_COUNT, 1.0f);
	}
	_reserved_beers.clear();
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_resource_exit_signal() {
	M2_LEVEL.enable_hud();
	M2_LEVEL.remove_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
	return std::nullopt;
}

std::optional<SellJourneyStep> SellJourney::handle_confirmation_enter_signal() {
	LOG_INFO("Asking for confirmation...");
	auto card_name = M2_GAME.get_named_item(_selected_card).in_game_name();
	auto city_name = M2_GAME.get_named_item(city_of_location(_selected_location)).in_game_name();
	auto industry = to_industry_of_factory_character(find_factory_at_location(_selected_location)->character());
	auto industry_name = M2_GAME.get_named_item(industry).in_game_name();
	if (ask_for_confirmation("Sell " + industry_name + " in " + city_name, "using " + card_name + " card?", "OK", "Cancel")) {
		LOG_INFO("Sell action confirmed");
		M2_LEVEL.display_message("Selling location...");

		m2g::pb::ClientCommand cc;
		cc.mutable_sell_action()->set_card(_selected_card);
		cc.mutable_sell_action()->set_industry_location(_selected_location);
		for (const auto& beer_source : _beer_sources) {
			cc.mutable_sell_action()->add_beer_sources(beer_source);
		}
		M2_GAME.client_thread().queue_client_command(cc);
	} else {
		LOG_INFO("Cancelling Sell action...");
		// Return the reserved resources
		for (auto* object : _reserved_beers) {
			object->character().add_resource(BEER_BARREL_COUNT, 1.0f);
		}
		_reserved_beers.clear();
	}
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}
