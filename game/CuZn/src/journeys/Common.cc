#include <cuzn/journeys/Common.h>
#include <m2g/Proxy.h>

using namespace m2;
using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

const m2::ui::Blueprint journey_cancel_button{
	.border_width_px = 0,
	.widgets = {
		WidgetBlueprint{
			.background_color = {0, 0, 0, 255},
			.variant = widget::TextBlueprint{
				.initial_text = "Cancel",
				.on_action = [](MAYBE const widget::Text& self) -> Action {
					// Create and send a cancel signal to the current user journey
					auto& user_journey = m2g::Proxy::get_instance().user_journey;
					// Deliver cancel signal to current Journey
					std::visit(m2::overloaded {
						[](auto& j) { j.signal(PositionOrCancelSignal::create_cancel_signal()); }
					}, *user_journey);
					return make_return_action();
				}
			}
		}
	}
};

PositionOrCancelSignal PositionOrCancelSignal::create_mouse_click_signal(m2::VecF world_position) {
	auto signal = PositionOrCancelSignal{m2::FsmSignalType::Custom};
	signal._world_position = world_position;
	return signal;
}

PositionOrCancelSignal PositionOrCancelSignal::create_cancel_signal(bool cancelled) {
	auto signal = PositionOrCancelSignal{m2::FsmSignalType::Custom};
	signal._cancel = cancelled;
	return signal;
}
