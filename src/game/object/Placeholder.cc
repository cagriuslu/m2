#include "m2/game/object/Placeholder.h"

m2::Id m2::obj::create_background_placeholder(const VecF& pos, const Sprite& sprite, BackgroundLayer layer) {
	auto it = create_object(pos);
	it->add_terrain_graphic(layer, sprite);
	return it.id();
}

m2::Id m2::obj::create_foreground_placeholder(const VecF& pos, const Sprite& sprite) {
	auto it = create_object(pos);
	it->add_graphic(sprite);
	return it.id();
}
