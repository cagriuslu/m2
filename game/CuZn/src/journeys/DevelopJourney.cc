#include <cuzn/journeys/DevelopJourney.h>
#include <m2/Log.h>
#include "m2/Game.h"
#include "cuzn/ui/Detail.h"
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Factory.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

DevelopJourney::DevelopJourney() : FsmBase() {
	LOG_DEBUG("DevelopJourney constructed");
	init(DevelopJourneyStep::INITIAL_STEP);
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_signal(const PositionOrCancelSignal& s) {
	switch (state()) {
		case DevelopJourneyStep::INITIAL_STEP:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_initial_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2ERROR("Unexpected signal");
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
					throw M2ERROR("Unexpected signal");
				}
				case FsmSignalType::ExitState: return handle_resource_exit_signal();
				default: throw M2ERROR("Unexpected signal");
			}
		case DevelopJourneyStep::EXPECT_CONFIRMATION:
			switch (s.type()) {
				case FsmSignalType::EnterState: return handle_confirmation_enter_signal();
				case FsmSignalType::ExitState: return std::nullopt;
				default: throw M2ERROR("Unexpected signal");
			}
	}
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_initial_enter_signal() {
	if (player_card_count(M2_PLAYER.character()) == 0) {
		throw M2ERROR("Player has no cards but DevelopJourney is triggered. The game should have ended instead");
	}
	if (player_tile_count(M2_PLAYER.character()) == 0) {
		M2_LEVEL.display_message("You are out of factory tiles.");
		LOG_INFO("Insufficient factory tiles, cancelling DevelopJourney...");
		deinit();
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}

	if (1 < player_tile_count(M2_PLAYER.character())) {
		_develop_double_tiles = ask_for_confirmation("Develop double tiles?", "", "Yes", "No");
	}

	// Card selection
	if (auto selected_card = ask_for_card_selection()) {
		_selected_card = *selected_card;
	} else {
		deinit();
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
				deinit();
				M2_DEFER(m2g::Proxy::user_journey_deleter);
				return std::nullopt;
			}
		}
	} else {
		deinit();
		M2_DEFER(m2g::Proxy::user_journey_deleter);
		return std::nullopt;
	}

	return DevelopJourneyStep::EXPECT_RESOURCE_SOURCE;
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_resource_enter_signal() {
	LOG_DEBUG("Expecting iron source...");
	M2_LEVEL.disable_hud();
	M2_LEVEL.add_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX, RectF{0.775f, 0.1f, 0.15f, 0.1f}, &journey_cancel_button);

	if (_develop_double_tiles) {
		M2_LEVEL.display_message("Pick the first iron source");
	} else {
		M2_LEVEL.display_message("Pick an iron source");
	}
	return std::nullopt;
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_resource_mouse_click_signal(const m2::VecF& world_position) {
	LOG_DEBUG("Received mouse click", world_position);

	if (auto industry_loc = industry_location_on_position(world_position)) {
		LOG_DEBUG("Industry location", m2g::pb::SpriteType_Name(*industry_loc));
		// Check if location has a built factory
		if (auto* factory = find_factory_at_location(*industry_loc)) {
			// Check if factory has the required resource
			if (m2::is_less_or_equal(1.0f, IRON_CUBE_COUNT, 0.001f)) {
				// Deduct resource
				factory->character().remove_resource(IRON_CUBE_COUNT, 1.0f);
				// Save source
				if (not _iron_source_1) {
					_iron_source_1 = *industry_loc;
					_reserved_source_1 = factory;
					if (_develop_double_tiles) {
						return std::nullopt;
					} else {
						return DevelopJourneyStep::EXPECT_CONFIRMATION;
					}
				} else if (_develop_double_tiles && not _iron_source_2) {
					_iron_source_2 = *industry_loc;
					_reserved_source_2 = factory;
					return DevelopJourneyStep::EXPECT_CONFIRMATION;
				} else {
					throw M2ERROR("Invalid state");
				}
			} else {
				LOG_DEBUG("Industry doesn't have the required resource");
			}
		} else {
			LOG_DEBUG("Industry is not built");
		}
	} else if (auto merchant_loc = merchant_location_on_position(world_position)) {
		LOG_DEBUG("Merchant location", m2g::pb::SpriteType_Name(*merchant_loc));
		// Save source
		if (not _iron_source_1) {
			_iron_source_1 = *merchant_loc;
			if (_develop_double_tiles) {
				return std::nullopt;
			} else {
				return DevelopJourneyStep::EXPECT_CONFIRMATION;
			}
		} else if (_develop_double_tiles && not _iron_source_2) {
			_iron_source_2 = *merchant_loc;
			return DevelopJourneyStep::EXPECT_CONFIRMATION;
		} else {
			throw M2ERROR("Invalid state");
		}
	}
	return std::nullopt;
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_resource_cancel_signal() {
	LOG_INFO("Cancelling Develop action...");
	// Return the reserved resources
	if (_reserved_source_1) {
		_reserved_source_1->character().add_resource(IRON_CUBE_COUNT, 1.0f);
		_reserved_source_1 = nullptr;
	}
	if (_reserved_source_2) {
		_reserved_source_2->character().add_resource(IRON_CUBE_COUNT, 1.0f);
		_reserved_source_2 = nullptr;
	}
	deinit();
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}

std::optional<DevelopJourneyStep> DevelopJourney::handle_resource_exit_signal() {
	M2_LEVEL.enable_hud();
	M2_LEVEL.remove_custom_ui(JOURNEY_CANCEL_BUTTON_CUSTOM_UI_INDEX);
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
	deinit();
	M2_DEFER(m2g::Proxy::user_journey_deleter);
	return std::nullopt;
}
