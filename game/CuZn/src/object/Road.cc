#include <cuzn/object/Road.h>
#include <m2/Game.h>

m2::Object* cuzn::find_road_at_location(m2g::pb::SpriteType location) {
	auto roads = LEVEL.characters
		| std::views::transform(m2::to_only_data)
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_road_character)
		| std::views::transform(m2::to_character_parent)
		| std::views::filter(m2::object_in_rect_filter(PROXY.network_positions[location]));
	if (auto road_it = roads.begin(); road_it != roads.end()) {
		return *road_it;
	}
	return nullptr;
}
