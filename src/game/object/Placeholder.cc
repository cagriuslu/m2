#include "m2/game/object/Placeholder.h"

m2::Id m2::obj::create_background_placeholder(const VecF& pos, m2g::pb::SpriteType spriteType, BackgroundLayer layer) {
	auto it = create_object(pos);
	it->add_terrain_graphic(layer, spriteType);
	return it.id();
}

m2::Id m2::obj::create_foreground_placeholder(const VecF& pos, float orientation, m2g::pb::SpriteType spriteType) {
	auto it = create_object(pos);
	it->orientation = orientation;
	it->add_graphic(spriteType);
	return it.id();
}
