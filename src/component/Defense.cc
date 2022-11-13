#include <m2/component/Defense.h>
#include <m2/Object.h>
#include <m2/Game.hh>

m2::Defense::Defense(Id object_id) : Component(object_id) {}

m2::Object& m2::Defense::parent() const {
	return *GAME.objects.get(object_id);
}
