#include <cuzn/object/Road.h>
#include <m2/Game.h>

m2::Object* find_road_at_location(m2g::pb::SpriteType location) {
	auto roads = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_road_character)
		| std::views::transform(m2::to_character_parent)
		| std::views::filter(m2::is_object_in_area(M2G_PROXY.connection_positions[location].second));
	if (auto road_it = roads.begin(); road_it != roads.end()) {
		return &*road_it;
	}
	return nullptr;
}
