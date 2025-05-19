#include "m2/game/object/God.h"
#include <m2/ui/widget/CheckboxWithText.h>
#include "m2/Game.h"
#include <m2/Log.h>

using namespace m2;

namespace {
	void HandleSecondaryButtonPress(const VecF& mousePosition) {
		std::visit(overloaded{
		        [=](level_editor::State& le) { le.HandleMousePrimaryButton(mousePosition); },
		        DEFAULT_OVERLOAD},
		    M2_LEVEL.stateVariant);
	}

	void HandlePrimaryButtonRelease(const VecF& firstPosition, const VecF& secondPosition) {
		std::visit(overloaded{
				[=](level_editor::State& le) { le.HandleMousePrimarySelectionComplete(firstPosition, secondPosition); },
				DEFAULT_OVERLOAD},
			M2_LEVEL.stateVariant);
	}
}  // namespace

Id obj::create_god() {
	const auto it = CreateObject({});

	it->AddPhysique().preStep = [](MAYBE Physique& phy) {
		auto& obj = phy.Owner();

		VecF move_direction;
		if (M2_GAME.events.IsKeyDown(m2g::pb::KeyType::MOVE_UP)) {
			move_direction.y -= 1.0f;
		}
		if (M2_GAME.events.IsKeyDown(m2g::pb::KeyType::MOVE_DOWN)) {
			move_direction.y += 1.0f;
		}
		if (M2_GAME.events.IsKeyDown(m2g::pb::KeyType::MOVE_LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (M2_GAME.events.IsKeyDown(m2g::pb::KeyType::MOVE_RIGHT)) {
			move_direction.x += 1.0f;
		}
		obj.position += move_direction.normalize() * (M2_GAME.DeltaTimeS() * M2_GAME.Dimensions().GameM().y);
		// Prevent God from going into negative quadrants
		obj.position = obj.position.clamp(VecF{0.0f, 0.0f}, std::nullopt);

		// Adjust zoom
		if (M2_GAME.events.PopKeyPress(m2g::pb::KeyType::ZOOM_OUT)) {
			M2_GAME.SetScale(M2_GAME.Dimensions().Scale() / 1.5f);
		}
		if (M2_GAME.events.PopKeyPress(m2g::pb::KeyType::ZOOM_IN)) {
			M2_GAME.SetScale(M2_GAME.Dimensions().Scale() * 1.5f);
		}

		if (const auto& mousePosition = M2_GAME.MousePositionWorldM(); not mousePosition.is_negative()) {
			if (M2_GAME.events.PopMouseButtonPress(MouseButton::SECONDARY)) {
				HandleSecondaryButtonPress(mousePosition);
			}
			if (M2_GAME.events.PopMouseButtonRelease(MouseButton::PRIMARY) && M2_LEVEL.PrimarySelection() && M2_LEVEL.PrimarySelection()->IsComplete()) {
				const auto [first, second] = *M2_LEVEL.PrimarySelection()->SelectionsM();
				HandlePrimaryButtonRelease(first, second);
			}
		}
	};

	it->AddGraphic().postDraw = [](MAYBE Graphic& gfx) {
		std::visit(overloaded{
		        [](const level_editor::State& le) { le.Draw(); },
		        [](const sheet_editor::State& se) { se.Draw(); },
		        [](const bulk_sheet_editor::State& se) { se.Draw(); },
		    	DEFAULT_OVERLOAD},
		    M2_LEVEL.stateVariant);
	};

	return it.GetId();
}
