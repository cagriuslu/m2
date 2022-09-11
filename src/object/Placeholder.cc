#include <m2/object/Placeholder.h>

std::pair<m2::Object&, m2::ID> m2::obj::create_placeholder(const Vec2f& pos, const Sprite& sprite, bool is_fg) {
	auto obj_pair = create_object(pos);
	auto& placeholder = obj_pair.first;

	if (is_fg) {
		placeholder.add_graphic(sprite);
	} else {
		placeholder.add_terrain_graphic(sprite);
	}

	return obj_pair;
}
