#include <m2/object/Ghost.h>
#include <m2/Game.hh>
#include <m2g/SpriteBlueprint.h>

std::pair<m2::Object&, m2::ID> m2::obj::create_ghost(const Sprite& sprite) {
	auto obj_pair = m2::create_object(GAME.mousePositionWRTGameWorld_m.round());
	auto& ghost = obj_pair.first;

	auto& gfx = ghost.add_graphic(sprite);

	auto& monitor = ghost.add_monitor();
	monitor.pre_phy = [&ghost](MAYBE m2::comp::Monitor& mon) {
		auto pos = GAME.mousePositionWRTGameWorld_m.round();
		ghost.position = pos;

		if (GAME.events.pop_mouse_button_press(m2::MouseButton::PRIMARY)) {
			// TODO
		}
	};

	return obj_pair;
}
