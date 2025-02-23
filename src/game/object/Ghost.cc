#include "m2/game/object/Ghost.h"

#include <m2/ui/widget/IntegerInput.h>

#include "m2/Game.h"

m2::Id m2::obj::create_ghost(const m2g::pb::SpriteType spriteType, const bool roundPositionToInteger) {
	const auto it = create_object({});
	if (roundPositionToInteger) {
		auto& gfx = it->add_graphic(spriteType);
		gfx.preDraw = [](const Graphic& gfx) {
			gfx.owner().position = M2_GAME.MousePositionWorldM().round();
			if (const auto* orientationInput = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::IntegerInput>("OrientationInput")) {
				gfx.owner().orientation = to_radians(orientationInput->value());
			}
		};
	} else {
		auto& gfx = it->add_graphic(spriteType);
		gfx.preDraw = [](const Graphic& gfx) {
			gfx.owner().position = M2_GAME.MousePositionWorldM();
			if (const auto* orientationInput = M2_LEVEL.RightHud()->find_first_widget_by_name<widget::IntegerInput>("OrientationInput")) {
				gfx.owner().orientation = to_radians(orientationInput->value());
			}
		};
	}
	return it.Id();
}
