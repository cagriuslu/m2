#include "m2/game/object/God.h"
#include <m2/ui/widget/CheckboxWithText.h>
#include "m2/Game.h"

using namespace m2;

namespace {
	void handle_primary_button_press(const VecF& mousePosition) {
		std::visit(overloaded{
		        [=](level_editor::State& le) { le.HandleMousePrimaryButton(mousePosition); },
		        [=](pixel_editor::State& pe) {
			        std::visit(overloaded{
			                [=](pixel_editor::State::PaintMode& v) { v.paint_color(mousePosition.iround()); },
			                [=](pixel_editor::State::EraseMode& v) { v.erase_color(mousePosition.iround()); },
			                [=](pixel_editor::State::ColorPickerMode& v) { v.pick_color(mousePosition.iround()); },
			                DEFAULT_OVERLOAD},
			            pe.mode);
		        },
		        DEFAULT_OVERLOAD},
		    M2_LEVEL.stateVariant);
	}

	void handle_secondary_button_press(const VecF& mousePosition) {
		if (std::holds_alternative<sheet_editor::State>(M2_LEVEL.stateVariant)) {
			std::visit(overloaded{
					[=](sheet_editor::State::ForegroundCompanionMode& v) { v.secondary_selection_position = mousePosition.hround(); },
					[=](sheet_editor::State::RectMode& v) { v.secondary_selection_position = mousePosition.hround(); },
					DEFAULT_OVERLOAD}, std::get<sheet_editor::State>(M2_LEVEL.stateVariant).mode);
		}
	}
}  // namespace

Id obj::create_god() {
	const auto it = create_object({});

	it->add_physique().pre_step = [](MAYBE Physique& phy) {
		auto& obj = phy.owner();

		VecF move_direction;
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
		obj.position += move_direction.normalize() * (M2_GAME.DeltaTimeS() * M2_GAME.Dimensions().GameM().y);
		// Prevent God from going into negative quadrants
		obj.position = obj.position.clamp(VecF{0.0f, 0.0f}, std::nullopt);

		// Adjust zoom
		if (M2_GAME.events.pop_key_press(Key::MINUS)) {
			M2_GAME.SetScale(M2_GAME.Dimensions().Scale() / 1.5f);
		}
		if (M2_GAME.events.pop_key_press(Key::PLUS)) {
			M2_GAME.SetScale(M2_GAME.Dimensions().Scale() * 1.5f);
		}

		if (const auto& mousePosition = M2_GAME.MousePositionWorldM(); not mousePosition.is_negative()) {
			if (M2_GAME.events.pop_mouse_button_press(MouseButton::PRIMARY)) {
				handle_primary_button_press(mousePosition);
			} else if (M2_GAME.events.pop_mouse_button_press(MouseButton::SECONDARY)) {
				handle_secondary_button_press(mousePosition);
			}
		}
	};

	it->add_graphic().post_draw = [](MAYBE Graphic& gfx) {
		std::visit(overloaded{
		        [](const level_editor::State& le) { le.Draw(); },
		        [](const sheet_editor::State& se) { se.Draw(); },
		        [](const bulk_sheet_editor::State& se) { se.Draw(); },
		    	DEFAULT_OVERLOAD},
		    M2_LEVEL.stateVariant);
	};

	return it.id();
}
