#include <rpg/fsm/DistanceKeeper.h>

rpg::DistanceKeeperFsm::DistanceKeeperFsm(const m2::Object* obj, const pb::Ai* ai) : FsmBase(DistanceKeeperMode::Idle), obj(obj), ai(ai), home_position(obj->position) {
	init();
}

std::optional<rpg::DistanceKeeperMode> rpg::DistanceKeeperFsm::handle_signal(const DistanceKeeperFsmSignal& s) {
	return {};
}
