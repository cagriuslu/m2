#include <cuzn/journeys/Common.h>

using namespace m2;
//using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;
using namespace cuzn;

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
