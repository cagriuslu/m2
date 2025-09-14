#include "m2/game/object/Ghost.h"

#include <m2/ui/widget/IntegerSelection.h>

#include "m2/Game.h"

m2::Id m2::obj::CreateGhost(const m2g::pb::SpriteType spriteType, const int roundToBin) {
	const auto it = CreateObject();
	auto& gfx = it->AddGraphic(pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, spriteType);
	if (roundToBin != 0) {
		gfx.preDraw = [roundToBin](Graphic& gfx, const Stopwatch::Duration&) {
			gfx.position = M2_GAME.MousePositionWorldM().RoundToBin(roundToBin);
			if (const auto* orientationInput = M2_LEVEL.GetRightHud()->FindWidget<widget::IntegerSelection>("OrientationInput")) {
				gfx.orientation = ToRadians(orientationInput->value());
			}
		};
	} else {
		gfx.preDraw = [](Graphic& gfx, const Stopwatch::Duration&) {
			gfx.position = M2_GAME.MousePositionWorldM();
			if (const auto* orientationInput = M2_LEVEL.GetRightHud()->FindWidget<widget::IntegerSelection>("OrientationInput")) {
				gfx.orientation = ToRadians(orientationInput->value());
			}
		};
	}
	return it.GetId();
}
