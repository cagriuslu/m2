#include "m2/game/object/Ghost.h"

#include <m2/ui/widget/IntegerInput.h>

#include "m2/Game.h"

m2::Id m2::obj::create_ghost(const m2g::pb::SpriteType spriteType, const int roundToBin) {
	const auto it = CreateObject({});
	if (roundToBin != 0) {
		auto& gfx = it->AddGraphic(spriteType);
		gfx.preDraw = [roundToBin](const Graphic& gfx) {
			gfx.Owner().position = M2_GAME.MousePositionWorldM().RoundToBin(roundToBin);
			if (const auto* orientationInput = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::IntegerInput>("OrientationInput")) {
				gfx.Owner().orientation = ToRadians(orientationInput->value());
			}
		};
	} else {
		auto& gfx = it->AddGraphic(spriteType);
		gfx.preDraw = [](const Graphic& gfx) {
			gfx.Owner().position = M2_GAME.MousePositionWorldM();
			if (const auto* orientationInput = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::IntegerInput>("OrientationInput")) {
				gfx.Owner().orientation = ToRadians(orientationInput->value());
			}
		};
	}
	return it.GetId();
}
