#include <m2/object/God.h>
#include <m2/Game.hh>

m2::ID m2::obj::create_god() {
	auto [obj, id] = create_object(Vec2f{});
	obj.impl = std::make_unique<m2::obj::God>();
	obj.add_monitor([&obj = obj](MAYBE m2::comp::Monitor& mon) {
		m2::Vec2f move_direction;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			move_direction.y -= 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			move_direction.y += 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			move_direction.x += 1.0f;
		}
		obj.position += move_direction.normalize() * ((float)GAME.deltaTicks_ms * .01f);

		if (GAME.level->editor_mode == Level::EditorMode::PAINT && GAME.events.pop_mouse_button_press(MouseButton::PRIMARY)) {
			auto mouse_coordinates = GAME.mousePositionWRTGameWorld_m.iround();
			if (mouse_coordinates.in_nonnegative()) {
				GAME.level->editor_paint_mode_paint_sprite(mouse_coordinates);
			}
		}
	});
	return id;
}
