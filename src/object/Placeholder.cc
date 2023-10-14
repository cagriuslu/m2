#include <m2/object/Placeholder.h>

m2::Id m2::obj::create_background_placeholder(const VecF& pos, const Sprite& sprite, BackgroundLayer layer) {
	auto [obj, id] = create_object(pos);
	obj.add_terrain_graphic(layer, sprite);
	return id;
}

m2::Id m2::obj::create_foreground_placeholder(const VecF& pos, const Sprite& sprite) {
	auto [obj, id] = create_object(pos);
	obj.add_graphic(sprite);
	return id;
}
