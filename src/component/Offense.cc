#include <m2/component/Offense.h>
#include <m2/Object.h>
#include <m2/Game.hh>

m2::comp::Offense::Offense(ID object_id) : Component(object_id) {}

m2::Object& m2::comp::Offense::parent() const {
	return *GAME.objects.get(object_id);
}
