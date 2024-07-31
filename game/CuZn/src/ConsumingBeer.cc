#include <cuzn/ConsumingBeer.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>
#include <cuzn/detail/Network.h>
#include <cuzn/object/Merchant.h>

std::set<Location> find_breweries_with_beer(m2::Character& player, City city, std::optional<MerchantLocation> selling_to) {
	std::set<Location> locations;

	// Look-up player's own breweries with beer
	auto player_breweries_with_beer = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(m2::is_component_of_parent(player.parent_id()))
		| std::views::filter(is_factory_character)
		| std::views::filter([](m2::Character& chr) { return chr.has_resource(m2g::pb::BEER_BARREL_COUNT); })
		| std::views::transform(to_industry_location_of_factory_character);
	locations.insert(player_breweries_with_beer.begin(), player_breweries_with_beer.end());

	// Look-up breweries reachable from the city
	auto reachable_breweries_with_beer = reachable_locations_from_industry_city(city)
		| std::views::filter(is_industry_location)
		| std::views::filter(find_factory_at_location)
		| std::views::transform(find_factory_at_location)
		| std::views::transform(m2::to_character_of_object_unsafe)
		| std::views::filter([](m2::Character& chr) { return chr.has_resource(m2g::pb::BEER_BARREL_COUNT); })
		| std::views::transform(to_industry_location_of_factory_character);
	locations.insert(reachable_breweries_with_beer.begin(), reachable_breweries_with_beer.end());

	// If selling to a merchant, check if the merchant location has beer
	if (selling_to) {
		// Check if merchant is active, has beer, and is connected
		if (auto merchant = find_merchant_at_location(*selling_to);
			merchant && merchant->character().has_resource(m2g::pb::BEER_BARREL_COUNT)
			&& is_industry_city_connected_to_location(city, *selling_to)) {
			locations.insert(*selling_to);
		}
	}

	return locations;
}
