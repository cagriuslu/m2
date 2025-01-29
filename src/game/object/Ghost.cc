#include "m2/game/object/Ghost.h"

#include "m2/Game.h"

m2::Id m2::obj::create_ghost(const m2g::pb::SpriteType spriteType, const bool roundPositionToInteger) {
	const auto it = create_object({});
	if (roundPositionToInteger) {
		it->add_graphic(spriteType).pre_draw = [](const Graphic& gfx) {
			gfx.owner().position = M2_GAME.MousePositionWorldM().round();
		};
	} else {
		it->add_graphic(spriteType).pre_draw = [](const Graphic& gfx) {
			gfx.owner().position = M2_GAME.MousePositionWorldM();
		};
	}
	return it.id();
}
