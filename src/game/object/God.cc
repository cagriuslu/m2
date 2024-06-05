#include "m2/game/object/God.h"

#include "m2/Game.h"
#include "m2/ui/widget/ImageSelection.h"
#include "m2/ui/widget/IntegerInput.h"
#include "m2/ui/widget/Text.h"
#include "m2/ui/widget/TextSelection.h"

using namespace m2;

namespace {
	void level_editor_pick_foreground(const pb::LevelObject& level_object) {
		// Find the Place button
		auto* widget = ui::find_text_widget(*M2_LEVEL.left_hud_ui_state, m2::level_editor::place_button_label.data());
		if (!widget) {
			return;
		}
		// Press the button
		dynamic_cast<ui::widget::Text*>(widget)->trigger_action();
		// Right hud points to `place_mode_right_hud`, select the object type
		auto object_type_index = 0;
		for (const auto& level_editor_object : M2_GAME.object_main_sprites) {
			if (level_editor_object.first == level_object.type()) {
				dynamic_cast<ui::widget::TextSelection&>(*M2_LEVEL.right_hud_ui_state->widgets[1])
				    .set_selection(object_type_index);
				break;
			}
			++object_type_index;
		}
		// Select group type
		auto group_type_index = pb::enum_index(level_object.group().type());
		dynamic_cast<ui::widget::TextSelection&>(*M2_LEVEL.right_hud_ui_state->widgets[2]).set_selection(group_type_index);
		// Select group instance
		auto group_instance = level_object.group().instance();
		dynamic_cast<ui::widget::IntegerInput&>(*M2_LEVEL.right_hud_ui_state->widgets[3]).select((int)group_instance);
	}

	void level_editor_pick_background(m2g::pb::SpriteType picked_sprite_type) {
		// Find the Place button
		auto* widget = ui::find_text_widget(*M2_LEVEL.left_hud_ui_state, m2::level_editor::paint_button_label.data());
		if (!widget) {
			return;
		}
		// Press the button
		dynamic_cast<ui::widget::Text*>(widget)->trigger_action();
		// Right hud points to `paint_mode_right_hud`, select the sprite type
		auto sprite_type_index = 0;
		for (const auto& sprite_type : M2_GAME.level_editor_background_sprites) {
			if (sprite_type == picked_sprite_type) {
				dynamic_cast<ui::widget::ImageSelection&>(*M2_LEVEL.right_hud_ui_state->widgets[2])
				    .select(sprite_type_index);
				break;
			}
			++sprite_type_index;
		}
	}

	void handle_primary_button_press(const VecI& mouse_coordinates_i) {
		std::visit(
		    overloaded{
		        [=](ledit::State& le) {
			        std::visit(
			            overloaded{
			                [=](ledit::State::PaintMode& v) { v.paint_sprite(mouse_coordinates_i); },
			                [=](ledit::State::EraseMode& v) { v.erase_position(mouse_coordinates_i); },
			                [=](ledit::State::PlaceMode& v) { v.place_object(mouse_coordinates_i); },
			                [=](MAYBE ledit::State::RemoveMode& v) {
				                ledit::State::RemoveMode::remove_object(mouse_coordinates_i);
			                },
			                [=](ledit::State::PickMode& v) {
				                if (v.pick_foreground) {
					                if (const auto level_object = v.lookup_foreground_object(mouse_coordinates_i);
					                    level_object) {
						                level_editor_pick_foreground(*level_object);
					                }
				                } else {
					                if (const auto picked_sprite_type = v.lookup_background_sprite(mouse_coordinates_i);
					                    picked_sprite_type) {
						                level_editor_pick_background(*picked_sprite_type);
					                }
				                }
			                },
			                [=](const ledit::State::ShiftMode& v) { v.shift(mouse_coordinates_i); }, DEFAULT_OVERLOAD},
			            le.mode);
		        },
		        [=](pedit::State& pe) {
			        std::visit(
			            overloaded{
			                [=](pedit::State::PaintMode& v) { v.paint_color(mouse_coordinates_i); },
			                [=](pedit::State::EraseMode& v) { v.erase_color(mouse_coordinates_i); },
			                [=](pedit::State::ColorPickerMode& v) { v.pick_color(mouse_coordinates_i); },
			                DEFAULT_OVERLOAD},
			            pe.mode);
		        },
		        DEFAULT_OVERLOAD},
		    M2_LEVEL.type_state);
	}

	void handle_secondary_button_press(MAYBE const VecI& mouse_coordinates_i, const VecF& mouse_coordinates_h) {
		std::visit(
		    overloaded{
		        [=](sedit::State& se) {
			        std::visit(
			            overloaded{
			                [=](sedit::State::ForegroundCompanionMode& v) {
				                v.secondary_selection_position = mouse_coordinates_h;
			                },
			                [=](sedit::State::RectMode& v) { v.secondary_selection_position = mouse_coordinates_h; },
			                DEFAULT_OVERLOAD},
			            se.mode);
		        },
		        DEFAULT_OVERLOAD},
		    M2_LEVEL.type_state);
	}

	void handle_mouse_events(const VecI& mouse_coordinates_i, const VecF& mouse_coordinates_h) {
		// Check if mouse is in positive quadrant
		if (mouse_coordinates_i.is_negative()) {
			return;
		}
		// Check if mouse pressed
		if (M2_GAME.events.pop_mouse_button_press(MouseButton::PRIMARY)) {
			handle_primary_button_press(mouse_coordinates_i);
		} else if (M2_GAME.events.pop_mouse_button_press(MouseButton::SECONDARY)) {
			handle_secondary_button_press(mouse_coordinates_i, mouse_coordinates_h);
		}
	}
}  // namespace

m2::Id m2::obj::create_god() {
	auto it = create_object({});
	it->impl = std::make_unique<God>();

	it->add_physique().pre_step = [](MAYBE Physique& phy) {
		auto& obj = phy.parent();

		m2::VecF move_direction;
		if (M2_GAME.events.is_key_down(Key::UP)) {
			move_direction.y -= 1.0f;
		}
		if (M2_GAME.events.is_key_down(Key::DOWN)) {
			move_direction.y += 1.0f;
		}
		if (M2_GAME.events.is_key_down(Key::LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (M2_GAME.events.is_key_down(Key::RIGHT)) {
			move_direction.x += 1.0f;
		}
		obj.position += move_direction.normalize() * ((float)M2_GAME.delta_time_s() * M2_GAME.dimensions().height_m);
		// Prevent God from going into negative quadrants
		obj.position = obj.position.clamp(VecF{0.0f, 0.0f}, std::nullopt);

		handle_mouse_events(M2_GAME.mouse_position_world_m().iround(), M2_GAME.mouse_position_world_m().hround());

		// Change zoom
		if (std::holds_alternative<sedit::State>(M2_LEVEL.type_state) ||
		    std::holds_alternative<bsedit::State>(M2_LEVEL.type_state)) {
			if (M2_GAME.events.pop_key_press(Key::MINUS)) {
				M2_GAME.set_zoom(1.1f);  // Increase game height
			} else if (M2_GAME.events.pop_key_press(Key::PLUS)) {
				M2_GAME.set_zoom(0.9f);  // Decrease game height
			}
		}
	};

	it->add_graphic().post_draw = [](MAYBE Graphic& gfx) {
		// Check if level editor select mode is active
		std::visit(
		    overloaded{
		        [](ledit::State& le) {
			        std::visit(
			            overloaded{[](const ledit::State::SelectMode& mode) { mode.on_draw(); }, DEFAULT_OVERLOAD},
			            le.mode);
		        },
		        [](sedit::State& se) {
			        std::visit(
			            overloaded{[](const auto& mode) { mode.on_draw(); }, [](MAYBE const std::monostate&) {}},
			            se.mode);
		        },
		        [](bsedit::State& se) { se.on_draw(); }, DEFAULT_OVERLOAD},
		    M2_LEVEL.type_state);
	};

	return it.id();
}
