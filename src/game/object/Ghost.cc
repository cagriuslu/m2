#include "m2/game/object/Ghost.h"

#include "m2/Game.h"

m2::Id m2::obj::create_ghost(const Sprite& sprite) {
	auto it = m2::create_object(M2_GAME.MousePositionWorldM().round());
	it->add_graphic(sprite);
	it->add_physique().pre_step = [](MAYBE Physique& phy) {
		phy.owner().position = M2_GAME.MousePositionWorldM().round();
	};
	return it.id();
}
