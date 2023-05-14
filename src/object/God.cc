#include <m2/object/God.h>
#include <m2/Game.h>

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color CLIPBOARD_COLOR = {0, 255, 0, 80};
}

m2::Id m2::obj::create_god() {
	auto [obj, id] = create_object(Vec2f{});
	obj.impl = std::make_unique<God>();

	obj.add_physique().pre_step = [&obj = obj](MAYBE Physique& phy) {
		m2::Vec2f move_direction;
		if (GAME.events.is_key_down(Key::UP)) {
			move_direction.y -= 1.0f;
		}
		if (GAME.events.is_key_down(Key::DOWN)) {
			move_direction.y += 1.0f;
		}
		if (GAME.events.is_key_down(Key::LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (GAME.events.is_key_down(Key::RIGHT)) {
			move_direction.x += 1.0f;
		}
		obj.position += move_direction.normalize() * ((float)GAME.delta_time_s() * 10.0f);

		// Check if mouse is in positive quadrant
		if (Vec2i mouse_coordinates = GAME.mouse_position_world_m().iround(); not mouse_coordinates.is_negative()) {
			// Check if mouse pressed
			if (GAME.events.pop_mouse_button_press(MouseButton::PRIMARY)) {
				LOG_DEBUG("Mouse pressed");
				switch (LEVEL.type()) {
					case Level::Type::LEVEL_EDITOR: {
						std::visit(m2::overloaded {
								[=](Level::LevelEditorState::PaintMode& v) { v.paint_sprite(mouse_coordinates); },
								[=](Level::LevelEditorState::EraseMode& v) { v.erase_position(mouse_coordinates); },
								[=](Level::LevelEditorState::PlaceMode& v) { v.place_object(mouse_coordinates); },
								[=](Level::LevelEditorState::RemoveMode& v) { v.remove_object(mouse_coordinates); },
								[=](Level::LevelEditorState::SelectMode& v) {
									v.selection_position_1 = mouse_coordinates;
									v.selection_position_2 = {};
									LOG_DEBUG("Selection position 1", *v.selection_position_1);
								},
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
			} else if (GAME.events.pop_mouse_button_release(MouseButton::PRIMARY)) {
				// If mouse is released
				switch (LEVEL.type()) {
					case Level::Type::LEVEL_EDITOR: {
						std::visit(m2::overloaded {
								[=](Level::LevelEditorState::SelectMode& v) {
									if (v.selection_position_1) {
										auto [low_position, high_position] = std::minmax(*v.selection_position_1, mouse_coordinates, Vec2iCompareTopLeftToBottomRight{});
										v.selection_position_1 = low_position;
										v.selection_position_2 = high_position;
										LOG_DEBUG("Selection positions", *v.selection_position_1, *v.selection_position_2);
									}
								},
								[](MAYBE auto& v) {}
						}, LEVEL.level_editor_state->mode);
						break;
					}
					case Level::Type::PIXEL_EDITOR:
						break;
					default:
						throw M2FATAL("God is created for an invalid level type");
				}
			}
		}
	};

	obj.add_graphic().post_draw = [](MAYBE m2::Graphic& gfx) {
		// Check if level editor is active
		if (LEVEL.type() == Level::Type::LEVEL_EDITOR) {
			// Check if select mode is active
			if (std::holds_alternative<Level::LevelEditorState::SelectMode>(LEVEL.level_editor_state->mode)) {
				auto& select_mode = std::get<Level::LevelEditorState::SelectMode>(LEVEL.level_editor_state->mode);
				// Draw selection
				if (select_mode.selection_position_1 && not select_mode.selection_position_2) {
					// If the mouse is in the first quadrant, color selection
					if (Vec2i mouse_coordinates = GAME.mouse_position_world_m().iround(); not mouse_coordinates.is_negative()) {
						select_mode.selection_position_1->for_each_cell_in_between(mouse_coordinates, [=](const Vec2i& cell) {
							Graphic::color_cell(cell, SELECTION_COLOR);
						});
					}
				} else if (select_mode.selection_position_1 && select_mode.selection_position_2) {
					// Selection is done, color selection
					select_mode.selection_position_1->for_each_cell_in_between(*select_mode.selection_position_2, [=](const Vec2i& cell) {
						Graphic::color_cell(cell, SELECTION_COLOR);
					});
				}
				// Draw clipboard
				if (select_mode.clipboard_position_1 && select_mode.clipboard_position_2) {
					select_mode.clipboard_position_1->for_each_cell_in_between(*select_mode.clipboard_position_2, [=](const Vec2i& cell) {
						Graphic::color_cell(cell, CLIPBOARD_COLOR);
					});
				}
			}
		}
	};

	return id;
}
