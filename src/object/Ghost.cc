#include <m2/object/Ghost.h>
#include <m2/Game.hh>
#include <m2g/SpriteBlueprint.h>

m2::Vec2f calculate_ghost_screen_position() {
	auto mouse_pos_m = GAME.mousePositionWRTGameWorld_m;
	auto floored_mouse_pos_m = m2::Vec2f{floor(mouse_pos_m.x), floor(mouse_pos_m.y)};
	return floored_mouse_pos_m + 0.5f;
}

m2::Vec2i calculate_ghost_integer_position() {
	auto mouse_pos_m = GAME.mousePositionWRTGameWorld_m;
	return m2::Vec2i{floor(mouse_pos_m.x), floor(mouse_pos_m.y)};
}

std::pair<m2::Object&, m2::ID> m2::obj::create_ghost(const Sprite& sprite) {
	auto obj_pair = m2::create_object(calculate_ghost_screen_position());
	auto& ghost = obj_pair.first;

	auto& gfx = ghost.add_graphic(sprite);

	auto& monitor = ghost.add_monitor();
	monitor.pre_phy = [&ghost](MAYBE m2::comp::Monitor& mon) {
		auto pos = calculate_ghost_screen_position();
		ghost.position = pos;

		if (GAME.events.pop_mouse_button_press(m2::MouseButton::PRIMARY)) {
			auto integer_pos = calculate_ghost_integer_position();
		}
	};

	return obj_pair;
}
