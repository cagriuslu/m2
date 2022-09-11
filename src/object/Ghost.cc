#include <m2/object/Ghost.h>
#include <m2/Game.hh>

m2::ID m2::obj::create_ghost(const Sprite& sprite) {
	auto [obj, id] = m2::create_object(GAME.mousePositionWRTGameWorld_m.round());
	obj.add_graphic(sprite);
	obj.add_monitor([&obj = obj](MAYBE m2::comp::Monitor& mon) {
		obj.position = GAME.mousePositionWRTGameWorld_m.round();
	});
	return id;
}
