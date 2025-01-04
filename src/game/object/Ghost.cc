#include "m2/game/object/Ghost.h"

#include "m2/Game.h"

m2::Id m2::obj::create_ghost(const m2g::pb::SpriteType spriteType) {
	const auto it = create_object(M2_GAME.MousePositionWorldM().round());
	it->add_graphic(spriteType);
	it->add_physique().pre_step = [](MAYBE const Physique& phy) {
		phy.owner().position = M2_GAME.MousePositionWorldM().round();
	};
	return it.id();
}
