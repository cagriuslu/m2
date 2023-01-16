#include <m2/object/Ghost.h>
#include <m2/Game.h>

m2::Id m2::obj::create_ghost(const Sprite& sprite) {
	auto [obj, id] = m2::create_object(GAME.mousePositionWRTGameWorld_m.round());
	obj.add_graphic(sprite);
	obj.add_physique().pre_step = [&obj = obj](MAYBE Physique& phy) {
		obj.position = GAME.mousePositionWRTGameWorld_m.round();
	};

	return id;
}
