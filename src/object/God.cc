#include <m2/object/God.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/Game.h>

using namespace m2;

namespace {
	void level_editor_pick_foreground(const pb::LevelObject& level_object) {
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
			if (level_editor_object.first == level_object.type()) {
				dynamic_cast<ui::widget::TextSelection&>(*LEVEL.right_hud_ui_state->widgets[1]).select(object_type_index);
				break;
			}
			++object_type_index;
		}
		// Select group type
		auto group_type_index = protobuf::enum_index(level_object.group().type());
		dynamic_cast<ui::widget::TextSelection&>(*LEVEL.right_hud_ui_state->widgets[2]).select(group_type_index);
		// Select group instance
		auto group_instance = level_object.group().instance();
		dynamic_cast<ui::widget::IntegerSelection&>(*LEVEL.right_hud_ui_state->widgets[3]).select((int)group_instance);
	}

	void level_editor_pick_background(m2g::pb::SpriteType picked_sprite_type) {
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
				dynamic_cast<ui::widget::ImageSelection&>(*LEVEL.right_hud_ui_state->widgets[2]).select(sprite_type_index);
				break;
			}
			++sprite_type_index;
		}
	}

	void handle_primary_button_press(const VecI& mouse_coordinates_i) {
		if (LEVEL.type() == Level::Type::LEVEL_EDITOR) {
			std::visit(m2::overloaded {
					[=](ledit::State::PaintMode& v) { v.paint_sprite(mouse_coordinates_i); },
					[=](ledit::State::EraseMode& v) { v.erase_position(mouse_coordinates_i); },
					[=](ledit::State::PlaceMode& v) { v.place_object(mouse_coordinates_i); },
					[=](ledit::State::RemoveMode& v) { v.remove_object(mouse_coordinates_i); },
					[=](ledit::State::PickMode& v) {
						if (v.pick_foreground) {
							if (auto level_object = v.lookup_foreground_object(mouse_coordinates_i); level_object) {
								level_editor_pick_foreground(*level_object);
							}
						} else {
							if (auto picked_sprite_type = v.lookup_background_sprite(mouse_coordinates_i); picked_sprite_type) {
								level_editor_pick_background(*picked_sprite_type);
							}
						}
					},
					[=](ledit::State::ShiftMode& v) { v.shift(mouse_coordinates_i); },
					[](MAYBE auto& v) {}
			}, LEVEL.level_editor_state->mode);
		} else if (LEVEL.type() == Level::Type::PIXEL_EDITOR) {
			std::visit(m2::overloaded {
					[=](pedit::State::PaintMode& v) { v.paint_color(mouse_coordinates_i); },
					[=](pedit::State::EraseMode& v) { v.erase_color(mouse_coordinates_i); },
					[=](pedit::State::ColorPickerMode& v) { v.pick_color(mouse_coordinates_i); },
					[](MAYBE auto& v) {}
			}, LEVEL.pixel_editor_state->mode);
		}
	}

	void handle_secondary_button_press(MAYBE const VecI& mouse_coordinates_i, const VecF& mouse_coordinates_h) {
		if (LEVEL.type() == Level::Type::SHEET_EDITOR) {
			std::visit(m2::overloaded {
					[=](sedit::State::ForegroundCompanionMode& v) {
						v.secondary_selection_position = mouse_coordinates_h;
					},
					[=](sedit::State::RectMode& v) {
						v.secondary_selection_position = mouse_coordinates_h;
					},
					[](MAYBE auto&) {}
			}, LEVEL.sheet_editor_state->mode);
		}
	}

	void handle_mouse_events(const VecI& mouse_coordinates_i, const VecF& mouse_coordinates_h) {
		// Check if mouse is in positive quadrant
		if (mouse_coordinates_i.is_negative()) {
			return;
		}
		// Check if mouse pressed
		if (GAME.events.pop_mouse_button_press(MouseButton::PRIMARY)) {
			handle_primary_button_press(mouse_coordinates_i);
		} else if (GAME.events.pop_mouse_button_press(MouseButton::SECONDARY)) {
			handle_secondary_button_press(mouse_coordinates_i, mouse_coordinates_h);
		}
	}
}

m2::Id m2::obj::create_god() {
	auto [obj, id] = create_object(VecF{});
	obj.impl = std::make_unique<God>();

	obj.add_physique().pre_step = [&o = obj](MAYBE Physique& phy) {
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
		o.position += move_direction.normalize() * ((float)GAME.delta_time_s() * 10.0f);
		// Prevent God from going into negative quadrants
		o.position = o.position.clamp(VecF{0.0f, 0.0f}, std::nullopt);

		handle_mouse_events(GAME.mouse_position_world_m().iround(), GAME.mouse_position_world_m().hround());

		// Change zoom
		if (LEVEL.type() == Level::Type::SHEET_EDITOR) {
			if (GAME.events.pop_key_press(Key::MINUS)) {
				GAME.set_zoom(1.1f); // Increase game height
			} else if (GAME.events.pop_key_press(Key::PLUS)) {
				GAME.set_zoom(0.9f); // Decrease game height
			}
		}
	};

	obj.add_graphic().post_draw = [](MAYBE m2::Graphic& gfx) {
		// Check if level editor select mode is active
		if (LEVEL.type() == Level::Type::LEVEL_EDITOR) {
			std::visit(m2::overloaded {
					[](const ledit::State::SelectMode& mode) { mode.on_draw(); },
					[](MAYBE const auto&) {}
			}, LEVEL.level_editor_state->mode);
		} else if (LEVEL.type() == Level::Type::SHEET_EDITOR) {
			std::visit(m2::overloaded {
					[](const auto& mode) { mode.on_draw(); },
					[](MAYBE const std::monostate&) {}
			}, LEVEL.sheet_editor_state->mode);
		}
	};

	return id;
}
