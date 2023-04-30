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
		obj.position += move_direction.normalize() * ((float)GAME.delta_time_s() * 10.0f);

		Vec2i mouse_coordinates;
		if (GAME.events.pop_mouse_button_press(MouseButton::PRIMARY) && !(mouse_coordinates = GAME.mouse_position_world_m().iround()).is_negative()) {
			switch (LEVEL.type()) {
				case Level::Type::LEVEL_EDITOR: {
					std::visit(m2::overloaded {
							[=](Level::LevelEditorState::PaintMode& v) { v.paint_sprite(mouse_coordinates); },
							[=](Level::LevelEditorState::EraseMode& v) { v.erase_position(mouse_coordinates); },
							[=](Level::LevelEditorState::PlaceMode& v) { v.place_object(mouse_coordinates); },
							[=](Level::LevelEditorState::RemoveMode& v) { v.remove_object(mouse_coordinates); },
							[=](Level::LevelEditorState::ShiftMode& v) { v.shift(mouse_coordinates); },
							[](MAYBE auto& v) {}
					}, LEVEL.level_editor_state->mode);
					break;
				}
				case Level::Type::PIXEL_EDITOR: {
					std::visit(m2::overloaded {
							[=](Level::PixelEditorState::PaintMode& v) { v.paint_color(mouse_coordinates); },
							[=](Level::PixelEditorState::EraseMode& v) { v.erase_color(mouse_coordinates); },
							[=](Level::PixelEditorState::ColorPickerMode& v) { v.pick_color(mouse_coordinates); },
							[](MAYBE auto& v) {}
					}, LEVEL.pixel_editor_state->mode);
					break;
				}
				default:
					throw M2FATAL("God is created for an invalid level type");
			}
		}
	};
	return id;
}
