#include <cuzn/object/Factory.h>
#include <m2/Game.h>

m2::Object* cuzn::find_factory_at_location(m2g::pb::SpriteType location) {
	auto factories = LEVEL.characters
		| std::views::transform(m2::to_only_data)
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_factory_character)
		| std::views::transform(m2::to_character_parent)
		| std::views::filter(m2::object_in_rect_filter(PROXY.industry_positions[location]));
	if (auto factory_it = factories.begin(); factory_it != factories.end()) {
		return *factory_it;
	}
	return nullptr;
}
