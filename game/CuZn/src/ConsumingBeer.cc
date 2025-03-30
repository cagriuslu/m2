#include <cuzn/ConsumingBeer.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>
#include <cuzn/detail/Network.h>
#include <cuzn/object/Merchant.h>

std::set<Location> find_breweries_with_beer(m2::Character& player, City city, std::optional<MerchantLocation> selling_to, City city_2) {
	std::set<Location> locations;

	// Look-up player's own breweries with beer
	auto player_breweries_with_beer = M2_LEVEL.characters
		| std::views::transform(m2::ToCharacterBase)
		| std::views::filter(m2::IsComponentOfAnyDescendant(player.OwnerId()))
		| std::views::filter(IsFactoryCharacter)
		| std::views::filter([](m2::Character& chr) { return chr.HasResource(m2g::pb::BEER_BARREL_COUNT); })
		| std::views::transform(ToIndustryLocationOfFactoryCharacter);
	locations.insert(player_breweries_with_beer.begin(), player_breweries_with_beer.end());

	// Look-up breweries reachable from the city
	auto reachable_locations = ReachableLocationsFromIndustryCity(city);
	if (city_2 && is_industry_city(city_2)) {
		auto reachable_locations_2 = ReachableLocationsFromIndustryCity(city_2);
		reachable_locations.insert(reachable_locations_2.begin(), reachable_locations_2.end());
	}
	auto reachable_breweries_with_beer = reachable_locations
		| std::views::filter(is_industry_location)
		| std::views::filter(FindFactoryAtLocation)
		| std::views::transform(FindFactoryAtLocation)
		| std::views::transform(m2::to_character_of_object_unsafe)
		| std::views::filter([](m2::Character& chr) { return chr.HasResource(m2g::pb::BEER_BARREL_COUNT); })
		| std::views::transform(ToIndustryLocationOfFactoryCharacter);
	locations.insert(reachable_breweries_with_beer.begin(), reachable_breweries_with_beer.end());

	// If selling to a merchant, check if the merchant location has beer
	if (selling_to) {
		// Check if merchant is active, has beer, and is connected
		if (auto merchant = find_merchant_at_location(*selling_to);
			merchant && merchant->GetCharacter().HasResource(m2g::pb::BEER_BARREL_COUNT)
			&& IsIndustryCityConnectedToLocation(city, *selling_to)) {
			locations.insert(*selling_to);
		}
	}

	return locations;
}
