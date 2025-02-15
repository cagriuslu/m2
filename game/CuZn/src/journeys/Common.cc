#include <cuzn/journeys/Common.h>
#include <m2g/Proxy.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2g;
using namespace m2g::pb;

namespace {
	const m2::UiPanelBlueprint journey_cancel_button{
		.name = "JourneyCancelButton",
		.border_width = 0,
		.widgets = {
			UiWidgetBlueprint{
				.background_color = {0, 0, 0, 255},
				.variant = widget::TextBlueprint{
					.text = "Cancel",
					.onAction = [](MAYBE const widget::Text& self) -> UiAction {
						// Create and send a cancel signal to the current user journey
						if (M2G_PROXY.main_journeys) {
							std::visit(m2::overloaded{
									[](auto& journey) { journey.sub_journey->signal(PositionOrCancelSignal::create_cancel_signal()); }
							}, *M2G_PROXY.main_journeys);
						}
						return MakeReturnAction();
					}
				}
			}
		}
	};
}

std::list<m2::UiPanel>::iterator AddCancelButton() {
	return M2_LEVEL.AddCustomNonblockingUiPanel(&journey_cancel_button, RectF{0.775f, 0.1f, 0.15f, 0.1f});
}

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
