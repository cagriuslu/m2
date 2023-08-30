#include <m2/object/God.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/Game.h>

using namespace m2;

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color CLIPBOARD_COLOR = {0, 255, 0, 80};

	void level_editor_pick_foreground(const VecI& mouse_position) {
		// Pick position
		auto level_object = ledit::State::PickMode::lookup_foreground_object(mouse_position);
		if (!level_object) {
			return;
		}
		// Find the Place button
		auto* widget = ui::find_text_widget(*LEVEL.left_hud_ui_state, m2::level_editor::place_button_label.data());
		if (!widget) {
			return;
		}
		// Press the button
		std::get<ui::widget::TextBlueprint>(widget->blueprint->variant).action_callback();
		// Right hud points to `place_mode_right_hud`, select the object type
		auto object_type_index = 0;
		for (const auto& level_editor_object : GAME.level_editor_object_sprites) {
			if (level_editor_object.first == level_object->type()) {
				dynamic_cast<ui::widget::TextSelection&>(*LEVEL.right_hud_ui_state->widgets[1]).select(object_type_index);
				break;
			}
			++object_type_index;
		}
		// Select group type
		auto group_type_index = protobuf::enum_index(level_object->group().type());
		dynamic_cast<ui::widget::TextSelection&>(*LEVEL.right_hud_ui_state->widgets[2]).select(group_type_index);
		// Select group instance
		auto group_instance = level_object->group().instance();
		dynamic_cast<ui::widget::IntegerSelection&>(*LEVEL.right_hud_ui_state->widgets[3]).select((int)group_instance);
	}

	void level_editor_pick_background(const VecI& mouse_position) {
		// Pick position
		auto picked_sprite_type = ledit::State::PickMode::lookup_background_sprite(mouse_position);
		if (!picked_sprite_type) {
			return;
		}
		// Find the Place button
		auto* widget = ui::find_text_widget(*LEVEL.left_hud_ui_state, m2::level_editor::paint_button_label.data());
		if (!widget) {
			return;
		}
		// Press the button
		std::get<ui::widget::TextBlueprint>(widget->blueprint->variant).action_callback();
		// Right hud points to `paint_mode_right_hud`, select the sprite type
		auto sprite_type_index = 0;
		for (const auto& sprite_type : GAME.level_editor_background_sprites) {
			if (sprite_type == picked_sprite_type) {
				dynamic_cast<ui::widget::ImageSelection&>(*LEVEL.right_hud_ui_state->widgets[1]).select(sprite_type_index);
				break;
			}
			++sprite_type_index;
		}
	}
}

m2::Id m2::obj::create_god() {
	auto [obj, id] = create_object(VecF{});
	obj.impl = std::make_unique<God>();

	obj.add_physique().pre_step = [&obj = obj](MAYBE Physique& phy) {
		m2::VecF move_direction;
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
		// Prevent God from going into negative quadrants
		obj.position = obj.position.clamp(VecF{0.0f, 0.0f}, std::nullopt);

		// Check if mouse is in positive quadrant
		if (VecI mouse_coordinates = GAME.mouse_position_world_m().iround(); not mouse_coordinates.is_negative()) {
			// Check if mouse pressed
			if (GAME.events.pop_mouse_button_press(MouseButton::PRIMARY)) {
				LOG_DEBUG("Mouse pressed");
				if (LEVEL.type() == Level::Type::LEVEL_EDITOR) {
					std::visit(m2::overloaded {
							[=](ledit::State::PaintMode& v) { v.paint_sprite(mouse_coordinates); },
							[=](ledit::State::EraseMode& v) { v.erase_position(mouse_coordinates); },
							[=](ledit::State::PlaceMode& v) { v.place_object(mouse_coordinates); },
							[=](ledit::State::RemoveMode& v) { v.remove_object(mouse_coordinates); },
							[=](ledit::State::PickMode& v) {
								v.pick_foreground ? level_editor_pick_foreground(mouse_coordinates) : level_editor_pick_background(mouse_coordinates);
							},
							[=](ledit::State::SelectMode& v) {
								v.selection_position_1 = mouse_coordinates;
								v.selection_position_2 = {};
								LOG_DEBUG("Selection position 1", *v.selection_position_1);
							},
							[=](ledit::State::ShiftMode& v) { v.shift(mouse_coordinates); },
							[](MAYBE auto& v) {}
					}, LEVEL.level_editor_state->mode);
				} else if (LEVEL.type() == Level::Type::PIXEL_EDITOR) {
					std::visit(m2::overloaded {
							[=](pedit::State::PaintMode& v) { v.paint_color(mouse_coordinates); },
							[=](pedit::State::EraseMode& v) { v.erase_color(mouse_coordinates); },
							[=](pedit::State::ColorPickerMode& v) { v.pick_color(mouse_coordinates); },
							[](MAYBE auto& v) {}
					}, LEVEL.pixel_editor_state->mode);
				}
			} else if (GAME.events.pop_mouse_button_release(MouseButton::PRIMARY)) {
				// If mouse is released
				switch (LEVEL.type()) {
					case Level::Type::LEVEL_EDITOR: {
						std::visit(m2::overloaded {
								[=](ledit::State::SelectMode& v) {
									if (v.selection_position_1) {
										auto [low_position, high_position] = std::minmax(*v.selection_position_1, mouse_coordinates, VecICompareTopLeftToBottomRight{});
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

		// Change zoom
		if (LEVEL.type() == Level::Type::SHEET_EDITOR) {
			if (GAME.events.pop_key_press(Key::MINUS)) {
				// Decrease game height
				GAME.recalculate_dimensions(GAME.dimensions().window.w, GAME.dimensions().window.h, GAME.dimensions().height_m + Rational(1,1));
			} else if (GAME.events.pop_key_press(Key::PLUS)) {
				// Increase game height
				GAME.recalculate_dimensions(GAME.dimensions().window.w, GAME.dimensions().window.h, GAME.dimensions().height_m + Rational(-1,1));
			}
		}
	};

	obj.add_graphic().post_draw = [](MAYBE m2::Graphic& gfx) {
		// Check if level editor is active
		if (LEVEL.type() == Level::Type::LEVEL_EDITOR) {
			// Check if select mode is active
			if (std::holds_alternative<ledit::State::SelectMode>(LEVEL.level_editor_state->mode)) {
				auto& select_mode = std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode);
				// Draw selection
				if (select_mode.selection_position_1 && not select_mode.selection_position_2) {
					// If the mouse is in the first quadrant, color selection
					if (VecI mouse_coordinates = GAME.mouse_position_world_m().iround(); not mouse_coordinates.is_negative()) {
						select_mode.selection_position_1->for_each_cell_in_between(mouse_coordinates, [=](const VecI& cell) {
							Graphic::color_cell(cell, SELECTION_COLOR);
						});
					}
				} else if (select_mode.selection_position_1 && select_mode.selection_position_2) {
					// Selection is done, color selection
					select_mode.selection_position_1->for_each_cell_in_between(*select_mode.selection_position_2, [=](const VecI& cell) {
						Graphic::color_cell(cell, SELECTION_COLOR);
					});
				}
				// Draw clipboard
				if (select_mode.clipboard_position_1 && select_mode.clipboard_position_2) {
					select_mode.clipboard_position_1->for_each_cell_in_between(*select_mode.clipboard_position_2, [=](const VecI& cell) {
						Graphic::color_cell(cell, CLIPBOARD_COLOR);
					});
				}
			}
		}
	};

	return id;
}
