#include <cuzn/ConsumingIron.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>

std::set<IndustryLocation> find_iron_industries_with_iron() {
	auto industry_locations = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_factory_character)
		| std::views::filter([](const auto& chr) {
				return chr.has_resource(m2g::pb::IRON_CUBE_COUNT);
			})
		| std::views::transform(to_industry_location_of_factory_character);
	return {industry_locations.begin(), industry_locations.end()};
}
