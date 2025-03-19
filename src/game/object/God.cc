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
}  // namespace

Id obj::create_god() {
	const auto it = create_object({});

	it->add_physique().preStep = [](MAYBE Physique& phy) {
		auto& obj = phy.owner();

		VecF move_direction;
		if (M2_GAME.events.is_key_down(m2g::pb::KeyType::MOVE_UP)) {
			move_direction.y -= 1.0f;
		}
		if (M2_GAME.events.is_key_down(m2g::pb::KeyType::MOVE_DOWN)) {
			move_direction.y += 1.0f;
		}
		if (M2_GAME.events.is_key_down(m2g::pb::KeyType::MOVE_LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (M2_GAME.events.is_key_down(m2g::pb::KeyType::MOVE_RIGHT)) {
			move_direction.x += 1.0f;
		}
		obj.position += move_direction.normalize() * (M2_GAME.DeltaTimeS() * M2_GAME.Dimensions().GameM().y);
		// Prevent God from going into negative quadrants
		obj.position = obj.position.clamp(VecF{0.0f, 0.0f}, std::nullopt);

		// Adjust zoom
		if (M2_GAME.events.pop_key_press(m2g::pb::KeyType::ZOOM_OUT)) {
			M2_GAME.SetScale(M2_GAME.Dimensions().Scale() / 1.5f);
		}
		if (M2_GAME.events.pop_key_press(m2g::pb::KeyType::ZOOM_IN)) {
			M2_GAME.SetScale(M2_GAME.Dimensions().Scale() * 1.5f);
		}

		if (const auto& mousePosition = M2_GAME.MousePositionWorldM(); not mousePosition.is_negative()) {
			if (M2_GAME.events.pop_mouse_button_press(MouseButton::PRIMARY)) {
				handle_primary_button_press(mousePosition);
			}
		}
	};

	it->add_graphic().postDraw = [](MAYBE Graphic& gfx) {
		std::visit(overloaded{
		        [](const level_editor::State& le) { le.Draw(); },
		        [](const sheet_editor::State& se) { se.Draw(); },
		        [](const bulk_sheet_editor::State& se) { se.Draw(); },
		    	DEFAULT_OVERLOAD},
		    M2_LEVEL.stateVariant);
	};

	return it.GetId();
}
