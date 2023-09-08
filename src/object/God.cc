#include <m2/object/God.h>
#include <m2/ui/widget/ImageSelection.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/IntegerSelection.h>
#include <m2/Game.h>

using namespace m2;

namespace {
	// The color of the selection while the selection is being made
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	// The color of the selection once it's confirmed. For level editor, this means the selection is copied to clipboard
	// For sheet editor this means the selection is set.
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
	constexpr SDL_Color CROSS_COLOR = {0, 127, 255, 255};
	constexpr SDL_Color CONFIRMED_CROSS_COLOR = {0, 255, 0, 255};

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

	void handle_primary_button_press(const VecI& mouse_coordinates_i, const VecF& mouse_coordinates_h) {
		if (LEVEL.type() == Level::Type::LEVEL_EDITOR) {
			std::visit(m2::overloaded {
					[=](ledit::State::PaintMode& v) { v.paint_sprite(mouse_coordinates_i); },
					[=](ledit::State::EraseMode& v) { v.erase_position(mouse_coordinates_i); },
					[=](ledit::State::PlaceMode& v) { v.place_object(mouse_coordinates_i); },
					[=](ledit::State::RemoveMode& v) { v.remove_object(mouse_coordinates_i); },
					[=](ledit::State::PickMode& v) {
						v.pick_foreground ? level_editor_pick_foreground(mouse_coordinates_i) : level_editor_pick_background(mouse_coordinates_i);
					},
					[=](ledit::State::SelectMode& v) {
						v.selection_position_1 = mouse_coordinates_i;
						v.selection_position_2 = {};
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
		} else if (LEVEL.type() == Level::Type::SHEET_EDITOR) {
			std::visit(m2::overloaded {
					[=](sedit::State::ForegroundCompanionMode& v) {
						v.primary_selection_position_1 = mouse_coordinates_i;
						v.primary_selection_position_2 = {};
					},
					[=](sedit::State::RectMode& v) {
						v.primary_selection_position_1 = mouse_coordinates_i;
						v.primary_selection_position_2 = {};
					},
					[=](sedit::State::BackgroundColliderMode& v) {
						v.primary_selection_position_1 = mouse_coordinates_h;
						v.primary_selection_position_2 = {};
						// Clear circle selection because background collider is either rectangle of circle
						v.secondary_selection_position_1 = {};
						v.secondary_selection_position_2 = {};
					},
					[=](sedit::State::ForegroundColliderMode& v) {
						v.primary_selection_position_1 = mouse_coordinates_h;
						v.primary_selection_position_2 = {};
						// Clear circle selection because background collider is either rectangle of circle
						v.secondary_selection_position_1 = {};
						v.secondary_selection_position_2 = {};
					},
					[](MAYBE auto&) {}
			}, LEVEL.sheet_editor_state->mode);
		}
	}

	void handle_primary_button_release(const VecI& mouse_coordinates_i, const VecF& mouse_coordinates_h) {
		if (LEVEL.type() == Level::Type::LEVEL_EDITOR) {
			std::visit(m2::overloaded {
					[=](ledit::State::SelectMode& v) {
						if (v.selection_position_1) {
							auto [low_position, high_position] = std::minmax(*v.selection_position_1, mouse_coordinates_i, VecICompareTopLeftToBottomRight{});
							v.selection_position_1 = low_position;
							v.selection_position_2 = high_position;
						}
					},
					[](MAYBE auto& v) {}
			}, LEVEL.level_editor_state->mode);
		} else {
			std::visit(m2::overloaded {
					[=](sedit::State::ForegroundCompanionMode& v) {
						auto [low_position, high_position] = std::minmax(*v.primary_selection_position_1, mouse_coordinates_i, VecICompareTopLeftToBottomRight{});
						v.primary_selection_position_1 = low_position;
						v.primary_selection_position_2 = high_position;
					},
					[=](sedit::State::RectMode& v) {
						auto [low_position, high_position] = std::minmax(*v.primary_selection_position_1, mouse_coordinates_i, VecICompareTopLeftToBottomRight{});
						v.primary_selection_position_1 = low_position;
						v.primary_selection_position_2 = high_position;
					},
					[=](sedit::State::BackgroundColliderMode& v) {
						auto [low_position, high_position] = std::minmax(*v.primary_selection_position_1, mouse_coordinates_h, VecFCompareTopLeftToBottomRight{});
						v.primary_selection_position_1 = low_position;
						v.primary_selection_position_2 = high_position;
					},
					[=](sedit::State::ForegroundColliderMode& v) {
						auto [low_position, high_position] = std::minmax(*v.primary_selection_position_1, mouse_coordinates_h, VecFCompareTopLeftToBottomRight{});
						v.primary_selection_position_1 = low_position;
						v.primary_selection_position_2 = high_position;
					},
					[](MAYBE auto&) {}
			}, LEVEL.sheet_editor_state->mode);
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
					[=](sedit::State::BackgroundColliderMode& v) {
						// Clear rectangle selection because background collider is either rectangle of circle
						v.primary_selection_position_1 = {};
						v.primary_selection_position_2 = {};
						v.secondary_selection_position_1 = mouse_coordinates_h;
						v.secondary_selection_position_2 = {};
					},
					[=](sedit::State::ForegroundColliderMode& v) {
						// Clear rectangle selection because background collider is either rectangle of circle
						v.primary_selection_position_1 = {};
						v.primary_selection_position_2 = {};
						v.secondary_selection_position_1 = mouse_coordinates_h;
						v.secondary_selection_position_2 = {};
					},
					[](MAYBE auto&) {}
			}, LEVEL.sheet_editor_state->mode);
		}
	}

	void handle_secondary_button_release(MAYBE const VecI& mouse_coordinates_i, const VecF& mouse_coordinates_h) {
		if (LEVEL.type() == Level::Type::SHEET_EDITOR) {
			std::visit(m2::overloaded {
					[=](sedit::State::BackgroundColliderMode& v) {
						auto [low_position, high_position] = std::minmax(*v.secondary_selection_position_1, mouse_coordinates_h, VecFCompareTopLeftToBottomRight{});
						v.secondary_selection_position_1 = low_position;
						v.secondary_selection_position_1 = high_position;
					},
					[=](sedit::State::ForegroundColliderMode& v) {
						auto [low_position, high_position] = std::minmax(*v.secondary_selection_position_1, mouse_coordinates_h, VecFCompareTopLeftToBottomRight{});
						v.secondary_selection_position_1 = low_position;
						v.secondary_selection_position_1 = high_position;
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
			handle_primary_button_press(mouse_coordinates_i, mouse_coordinates_h);
		} else if (GAME.events.pop_mouse_button_release(MouseButton::PRIMARY)) {
			handle_primary_button_release(mouse_coordinates_i, mouse_coordinates_h);
		} else if (GAME.events.pop_mouse_button_press(MouseButton::SECONDARY)) {
			handle_secondary_button_press(mouse_coordinates_i, mouse_coordinates_h);
		} else if (GAME.events.pop_mouse_button_release(MouseButton::SECONDARY)) {
			handle_secondary_button_release(mouse_coordinates_i, mouse_coordinates_h);
		}
	}

	void draw_selection(const std::optional<VecI>& position_1, const std::optional<VecI>& position_2) {
		// Draw if only first position is available
		if (position_1) {
			// If the second position is not available, draw towards the mouse
			auto second_position = position_2 ? *position_2 : GAME.mouse_position_world_m().iround();
			if (not second_position.is_negative()) {
				position_1->for_each_cell_in_between(second_position, [=](const VecI& cell) {
					Graphic::color_cell(cell, SELECTION_COLOR);
				});
			}
		}
	}
	void draw_selection(const std::optional<VecF>& position_1, const std::optional<VecF>& position_2) {
		// Draw if only first position is available
		if (position_1) {
			// If the second position is not available, draw towards the mouse
			auto second_position = position_2 ? *position_2 : GAME.mouse_position_world_m();
			if (not second_position.is_negative()) {
				Graphic::color_rect(RectF::from_corners(*position_1, second_position), SELECTION_COLOR);
			}
		}
	}

	void draw_confirmed_selection(const std::optional<VecI>& position_1, const std::optional<VecI>& position_2) {
		if (position_1 && position_2) {
			position_1->for_each_cell_in_between(*position_2, [=](const VecI& cell) {
				Graphic::color_cell(cell, CONFIRMED_SELECTION_COLOR);
			});
		}
	}

	void draw_level_editor_select_mode() {
		const auto& select_mode = std::get<ledit::State::SelectMode>(LEVEL.level_editor_state->mode);
		// Draw selection
		draw_selection(select_mode.selection_position_1, select_mode.selection_position_2);
		// Draw clipboard
		draw_confirmed_selection(select_mode.clipboard_position_1, select_mode.clipboard_position_2);
	}

	void draw_sheet_editor_foreground_companion_mode() {
		const auto& fcomp_mode = std::get<sedit::State::ForegroundCompanionMode>(LEVEL.sheet_editor_state->mode);
		// Draw selection
		draw_selection(fcomp_mode.primary_selection_position_1, fcomp_mode.primary_selection_position_2);
		// Draw center selection
		if (fcomp_mode.secondary_selection_position) {
			Graphic::draw_cross(*fcomp_mode.secondary_selection_position, CROSS_COLOR);
		}

		for (const auto& rect : fcomp_mode.current_rects) {
			Graphic::color_rect(RectF{rect}.shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
		}
		if (fcomp_mode.current_center) {
			auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
			Graphic::draw_cross(sprite_center + *fcomp_mode.current_center, CONFIRMED_CROSS_COLOR);
		}
	}

	void draw_sheet_editor_rect_mode() {
		const auto& rect_mode = std::get<sedit::State::RectMode>(LEVEL.sheet_editor_state->mode);
		// Draw selection
		draw_selection(rect_mode.primary_selection_position_1, rect_mode.primary_selection_position_2);
		// Draw center selection
		if (rect_mode.secondary_selection_position) {
			Graphic::draw_cross(*rect_mode.secondary_selection_position, CROSS_COLOR);
		}

		if (rect_mode.current_rect) {
			Graphic::color_rect(RectF{*rect_mode.current_rect}.shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
		}
		if (rect_mode.current_center) {
			auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
			Graphic::draw_cross(sprite_center + *rect_mode.current_center, CONFIRMED_CROSS_COLOR);
		}
	}

	void draw_sheet_editor_background_collider_mode() {
		const auto& bcoll_mode = std::get<sedit::State::BackgroundColliderMode>(LEVEL.sheet_editor_state->mode);
		// Draw rect selection
		draw_selection(bcoll_mode.primary_selection_position_1, bcoll_mode.primary_selection_position_2);
		// Draw circ selection
		draw_selection(bcoll_mode.secondary_selection_position_1, bcoll_mode.secondary_selection_position_2);

		if (bcoll_mode.current_rect) {
			auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
			Graphic::color_rect(bcoll_mode.current_rect->shift(sprite_center), CONFIRMED_SELECTION_COLOR);
		}
		if (bcoll_mode.current_circ) {
			auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
			Graphic::color_rect(bcoll_mode.current_circ->shift(sprite_center), CONFIRMED_SELECTION_COLOR);
		}
	}

	void draw_sheet_editor_foreground_collider_mode() {
		const auto& fcoll_mode = std::get<sedit::State::ForegroundColliderMode>(LEVEL.sheet_editor_state->mode);
		// Draw rect selection
		draw_selection(fcoll_mode.primary_selection_position_1, fcoll_mode.primary_selection_position_2);
		// Draw circ selection
		draw_selection(fcoll_mode.secondary_selection_position_1, fcoll_mode.secondary_selection_position_2);

		if (fcoll_mode.current_rect) {
			auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
			Graphic::color_rect(fcoll_mode.current_rect->shift(sprite_center), CONFIRMED_SELECTION_COLOR);
		}
		if (fcoll_mode.current_circ) {
			auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
			Graphic::color_rect(fcoll_mode.current_circ->shift(sprite_center), CONFIRMED_SELECTION_COLOR);
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

		handle_mouse_events(GAME.mouse_position_world_m().iround(), GAME.mouse_position_world_m().hround());

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
		// Check if level editor select mode is active
		if (LEVEL.type() == Level::Type::LEVEL_EDITOR && std::holds_alternative<ledit::State::SelectMode>(LEVEL.level_editor_state->mode)) {
			draw_level_editor_select_mode();
		} else if (LEVEL.type() == Level::Type::SHEET_EDITOR) {
			if (std::holds_alternative<sedit::State::ForegroundCompanionMode>(LEVEL.sheet_editor_state->mode)) {
				draw_sheet_editor_foreground_companion_mode();
			} else if (std::holds_alternative<sedit::State::RectMode>(LEVEL.sheet_editor_state->mode)) {
				draw_sheet_editor_rect_mode();
			} else if (std::holds_alternative<sedit::State::BackgroundColliderMode>(LEVEL.sheet_editor_state->mode)) {
				draw_sheet_editor_background_collider_mode();
			} else if (std::holds_alternative<sedit::State::ForegroundColliderMode>(LEVEL.sheet_editor_state->mode)) {
				draw_sheet_editor_foreground_collider_mode();
			}
		}
	};

	return id;
}
