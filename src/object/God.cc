#include <m2/object/God.h>
#include <m2/Game.h>

m2::Id m2::obj::create_god() {
	auto [obj, id] = create_object(Vec2f{});
	obj.impl = std::make_unique<m2::obj::God>();
	obj.add_physique().pre_step = [&obj = obj](MAYBE Physique& mon) {
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
		obj.position += move_direction.normalize() * ((float)GAME.deltaTime_s * 10.0f);

		Vec2i mouse_coordinates;
		if (LEVEL.editor_mode != Level::EditorMode::NONE && GAME.events.pop_mouse_button_press(MouseButton::PRIMARY) &&
			!(mouse_coordinates = GAME.mousePositionWRTGameWorld_m.iround()).is_negative()) {
			switch (LEVEL.editor_mode) {
				case Level::EditorMode::PAINT:
					LEVEL.editor_paint_mode_paint_sprite(mouse_coordinates);
					break;
				case Level::EditorMode::ERASE:
					LEVEL.editor_erase_mode_erase_position(mouse_coordinates);
					break;
				case Level::EditorMode::PLACE:
					LEVEL.editor_place_mode_place_object(mouse_coordinates);
					break;
				case Level::EditorMode::REMOVE:
					LEVEL.editor_remove_mode_remove_object(mouse_coordinates);
					break;
				default:
					break;
			}
		}
	};
	return id;
}
