#include <cuzn/journeys/subjourneys/POISelectionJourney.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>

namespace {
	void notify_main_journey(std::optional<POI> poi) {
		std::visit(m2::overloaded{
			[=](auto& j) { j.signal(POIOrCancelSignal{poi}); }
		}, *M2G_PROXY.main_journeys);
	}
}

std::optional<POISelectionJourneyStep> POISelectionJourney::handle_signal(const PositionOrCancelSignal& signal) {
	switch (signal.type()) {
		case m2::FsmSignalType::EnterState: {
			// Look-up the object IDs of the POIs
			auto object_ids = _pois | std::views::transform([](POI poi) {
				if (is_industry_location(poi)) {
					// Light up the factory if built, otherwise the background sprite.
					if (auto* factory = find_factory_at_location(poi)) {
						return factory->id();
					} else {
						return std::get<m2::ObjectId>(M2G_PROXY.industry_positions[poi]);
					}
				} else if (is_merchant_location(poi)) {
					return std::get<m2::ObjectId>(M2G_PROXY.merchant_positions[poi]);
				} else if (is_connection(poi)) {
					return std::get<m2::ObjectId>(M2G_PROXY.connection_positions[poi]);
				} else {
					throw M2_ERROR("Unknown location");
				}
			});
			// Enable dimming with exceptions
			M2_GAME.enable_dimming_with_exceptions({object_ids.begin(), object_ids.end()});
			// Disable HUD
			M2_LEVEL.disable_hud();
			// Display message
			M2_LEVEL.display_message(_message);
			// Display cancel button
			_cancel_button_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(m2::ui::Panel{&journey_cancel_button, m2::RectF{0.775f, 0.1f, 0.15f, 0.1f}});
			break;
		}
		case m2::FsmSignalType::Custom: {
			if (auto world_position = signal.world_position()) {
				LOG_DEBUG("Received mouse click", *world_position);
				if (auto industry_location = industry_location_on_position(*world_position)) {
					// Look up factory if exists, otherwise the background sprite
					if (auto* factory = find_factory_at_location(*industry_location);
						(factory && M2_GAME.dimming_exceptions()->contains(factory->id()))
						|| M2_GAME.dimming_exceptions()->contains(std::get<m2::ObjectId>(M2G_PROXY.industry_positions[*industry_location]))) {
						notify_main_journey(*industry_location);
					}
				} else if (auto merchant_location = merchant_location_on_position(*world_position);
					merchant_location && M2_GAME.dimming_exceptions()->contains(std::get<m2::ObjectId>(M2G_PROXY.merchant_positions[*merchant_location]))) {
					notify_main_journey(*merchant_location);
				} else if (auto connection = connection_on_position(*world_position);
					connection && M2_GAME.dimming_exceptions()->contains(std::get<m2::ObjectId>(M2G_PROXY.connection_positions[*connection]))) {
					notify_main_journey(*connection);
				}
			} else if (signal.cancel()) {
				notify_main_journey(std::nullopt);
			}
			break;
		}
		case m2::FsmSignalType::ExitState: {
			if (_cancel_button_panel) {
				M2_LEVEL.remove_custom_nonblocking_ui_panel(*_cancel_button_panel);
				_cancel_button_panel.reset();
			}
			M2_LEVEL.remove_message();
			M2_LEVEL.enable_hud();
			M2_GAME.disable_dimming_with_exceptions();
			break;
		}
		default: break;
	}
	return std::nullopt;
}
