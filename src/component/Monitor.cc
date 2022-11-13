#include <m2/component/Monitor.h>
#include <m2/Object.h>
#include <m2/Game.hh>

m2::Monitor::Monitor(Id object_id) : Component(object_id) {}

m2::Monitor::Monitor(uint64_t object_id, const Callback& pre_phy, const Callback& post_phy, const Callback& pre_gfx, const Callback& post_gfx) : Component(object_id), pre_phy(pre_phy), post_phy(post_phy), pre_gfx(pre_gfx), post_gfx(post_gfx) {}

m2::Object& m2::Monitor::parent() const {
	return *GAME.objects.get(object_id);
}
