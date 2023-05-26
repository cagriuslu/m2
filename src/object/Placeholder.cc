#include <m2/object/Placeholder.h>

m2::Id m2::obj::create_placeholder(const VecF& pos, const Sprite& sprite, bool is_fg) {
	auto [obj, id] = create_object(pos);
	if (is_fg) {
		obj.add_graphic(sprite);
	} else {
		obj.add_terrain_graphic(sprite);
	}
	return id;
}
