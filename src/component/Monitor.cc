#include <m2/component/Monitor.h>
#include <m2/Object.h>
#include <m2/Game.hh>

m2::comp::Monitor::Monitor(ID object_id) : Component(object_id) {}

m2::Object& m2::comp::Monitor::parent() const {
	return *GAME.objects.get(object_id);
}
