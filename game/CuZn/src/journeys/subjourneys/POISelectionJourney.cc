#include <cuzn/journeys/subjourneys/POISelectionJourney.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>

namespace {
	void notify_main_journey(std::optional<POI> poi) {
		M2_DEFER([=]() {
			std::visit(m2::overloaded{
					[=](auto& j) { j.signal(POIOrCancelSignal{poi}); }
			}, *M2G_PROXY.main_journeys);
		});
	}
}

std::optional<POISelectionJourneyStep> POISelectionJourney::HandleSignal(const PositionOrCancelSignal& signal) {
	switch (signal.type()) {
		case m2::FsmSignalType::EnterState: {
			LOG_INFO("Entering point-of-interest selection journey");
			// Look-up the object IDs of the POIs
			auto object_ids = _pois | std::views::transform([](const POI poi) {
				if (is_industry_location(poi)) {
					// Light up the factory if built, otherwise the background sprite.
					if (const auto* factory = FindFactoryAtLocation(poi)) {
						return factory->id();
					}
					return std::get<m2::ObjectId>(M2G_PROXY.industry_positions[poi]);
				}
				if (is_merchant_location(poi)) {
					return std::get<m2::ObjectId>(M2G_PROXY.merchant_positions[poi]);
				}
				if (is_connection(poi)) {
					return std::get<m2::ObjectId>(M2G_PROXY.connection_positions[poi]);
				}
				throw M2_ERROR("Unknown location");
			});
			// Enable dimming with exceptions
			M2_LEVEL.enable_dimming_with_exceptions({object_ids.begin(), object_ids.end()});
			// Disable HUD
			M2_LEVEL.DisableHud();
			// Destroy cards panel if exists
			if (M2G_PROXY.cards_panel) {
				M2_LEVEL.remove_custom_nonblocking_ui_panel(*M2G_PROXY.cards_panel);
				M2G_PROXY.cards_panel.reset();
			}
			// Display message
			M2_LEVEL.ShowMessage(_message);
			if (_allow_cancellation) {
				// Display cancel button
				_cancel_button_panel = AddCancelButton();
			}
			break;
		}
		case m2::FsmSignalType::Custom: {
			if (const auto world_position = signal.world_position()) {
				if (const auto industry_location = industry_location_on_position(*world_position)) {
					// Look up factory if exists, otherwise the background sprite
					if (auto* factory = FindFactoryAtLocation(*industry_location);
						(factory && M2_LEVEL.dimming_exceptions()->contains(factory->id()))
						|| M2_LEVEL.dimming_exceptions()->contains(std::get<m2::ObjectId>(M2G_PROXY.industry_positions[*industry_location]))) {
						LOG_INFO("Player selected industry location of interest, notifying main journey", *industry_location);
						notify_main_journey(*industry_location);
					} else {
						LOG_INFO("Player selected uninteresting industry location", *industry_location);
					}
				} else if (auto merchant_location = merchant_location_on_position(*world_position);
					merchant_location && M2_LEVEL.dimming_exceptions()->contains(std::get<m2::ObjectId>(M2G_PROXY.merchant_positions[*merchant_location]))) {
					LOG_INFO("Player selected merchant location of interest, notifying main journey", *merchant_location);
					notify_main_journey(*merchant_location);
				} else if (auto connection = connection_on_position(*world_position);
					connection && M2_LEVEL.dimming_exceptions()->contains(std::get<m2::ObjectId>(M2G_PROXY.connection_positions[*connection]))) {
					LOG_INFO("Player selected connection of interest, notifying main journey", *connection);
					notify_main_journey(*connection);
				}
			} else if (signal.cancel()) {
				LOG_INFO("Player cancelled selection, notifying main journey");
				notify_main_journey(std::nullopt);
			} else {
				throw M2_ERROR("Unexpected signal");
			}
			break;
		}
		case m2::FsmSignalType::ExitState: {
			LOG_INFO("Exiting point-of-interest selection journey");
			if (_cancel_button_panel) {
				M2_LEVEL.remove_custom_nonblocking_ui_panel(*_cancel_button_panel);
				_cancel_button_panel.reset();
			}
			// M2G_PROXY.remove_notification(); // TODO both error messages and usage tips are shown as notification. We need to differentiate between the two before removing the notification.
			M2_LEVEL.EnableHud();
			M2_LEVEL.disable_dimming_with_exceptions();
			break;
		}
		default: break;
	}
	return std::nullopt;
}
