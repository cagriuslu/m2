#include <cuzn/ConsumingIron.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>

std::set<IndustryLocation> find_iron_industries_with_iron() {
	auto industry_locations = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(IsFactoryCharacter)
		| std::views::filter([](const auto& chr) {
				return chr.has_resource(m2g::pb::IRON_CUBE_COUNT);
			})
		| std::views::transform(ToIndustryLocationOfFactoryCharacter);
	return {industry_locations.begin(), industry_locations.end()};
}

bool is_there_iron_on_the_board() {
	// Iterate over factories
	auto there_is_iron_works_with_iron = std::ranges::any_of(
			M2_LEVEL.characters
				| std::views::transform(m2::to_character_base)
				| std::views::filter(IsFactoryCharacter),
			[](m2::Character& chr) {
				return m2::is_positive(chr.get_resource(m2g::pb::IRON_CUBE_COUNT), 0.001f);
			});
	// Check the market as well
	return there_is_iron_works_with_iron
		|| m2::is_positive(M2G_PROXY.game_state_tracker().get_resource(m2g::pb::IRON_CUBE_COUNT), 0.001f);
}
