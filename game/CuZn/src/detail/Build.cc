#include <cuzn/detail/Build.h>
#include <cuzn/Detail.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>

using namespace m2g;
using namespace m2g::pb;

std::vector<cuzn::Industry> cuzn::buildable_industries(m2g::pb::ItemType selected_card, m2g::pb::SpriteType selected_location) {
	if (not is_card(selected_card)) {
		throw M2ERROR("Item is not a card");
	}
	if (not is_industry_location(selected_location)) {
		throw M2ERROR("Sprite is not an industry location");
	}

	const auto& selected_card_item = M2_GAME.get_named_item(selected_card);
	const auto& selected_sprite_sprite = M2_GAME.get_sprite(selected_location);
	// Lookup industries on the sprite
	std::vector<ItemType> selected_sprite_industries;
	std::ranges::copy_if(selected_sprite_sprite.named_items(), std::back_inserter(selected_sprite_industries), [](auto item_type) {
		return (M2_GAME.get_named_item(item_type).category() == ITEM_CATEGORY_INDUSTRY_CARD);
	});
	if (selected_sprite_industries.empty()) {
		throw M2ERROR("Selected sprite does not hold any industry cards");
	}
	// Look up the location of the sprite
	auto location_card_it = std::ranges::find_if(selected_sprite_sprite.named_items(), [](auto item_type) {
		return (M2_GAME.get_named_item(item_type).category() == ITEM_CATEGORY_CITY_CARD);
	});
	if (location_card_it == selected_sprite_sprite.named_items().end()) {
		throw M2ERROR("Selected sprite does not hold a location card");
	}
	ItemType selected_sprite_location = *location_card_it;

	// If the card is wild card
	if (selected_card_item.category() == ITEM_CATEGORY_WILD_CARD) {
		// Any industry in the selected location can be built
		return selected_sprite_industries;
	} else if (selected_card_item.category() == ITEM_CATEGORY_INDUSTRY_CARD) {
		// Check if the selected industry exists in the sprite's industries
		auto industry_card_it = std::find(selected_sprite_industries.begin(), selected_sprite_industries.end(), selected_card);
		if (industry_card_it == selected_sprite_industries.end()) {
			return {}; // No buildable industries
		}
		return {selected_card}; // Only the selected industry card is buildable
	} else { // ITEM_CATEGORY_CITY_CARD
		// Check if the card belongs to this location
		if (selected_card == selected_sprite_location) {
			// Any industry in the selected location can be built
			return selected_sprite_industries;
		}
		return {}; // No buildable industries
	}
}

m2::expected<ItemType> cuzn::can_player_build_industry(m2::Character& player, ItemType card, SpriteType location, ItemType industry) {
	if (not player_has_card(player, card)) {
		return m2::make_unexpected("Player doesn't hold the given card");
	}
	if (find_factory_at_location(location)) {
		return m2::make_unexpected("Location already has a factory built");
	}
	if (not location_has_industry(location, industry)) {
		return m2::make_unexpected("Location does not contain the industry");
	}

	auto buildable_industries = cuzn::buildable_industries(card, location);
	if (std::ranges::find(buildable_industries, industry) == buildable_industries.end()) {
		return m2::make_unexpected("Selected card cannot build the industry on given location");
	}

	auto city = city_of_location(location);
	// Check if this location is part of the player's network, or they don't have a network.
	if (auto cities_in_network = get_cities_in_network(player); not cities_in_network.empty() && not cities_in_network.contains(city)) {
		return m2::make_unexpected("City is not in player's network");
	}

	// Find all the locations in the city
	auto other_locations_in_city = locations_in_city(city);
	// Remove the given location
	other_locations_in_city.erase(std::remove(other_locations_in_city.begin(), other_locations_in_city.end(), location), other_locations_in_city.end());

	// If there's more than one industry on this location, check if there's another location in the city with only this industry.
	// The location with only one industry must be built before building the factory on a multi-industry location.
	auto industries = industries_on_location(location);
	if (1 < industries.size()) {
		auto other_locations_with_only_the_industry = other_locations_in_city
			| std::views::filter([industry](SpriteType loc) {
				const auto industries = industries_on_location(loc);
				return (industries.size() == 1 && industries[0] == industry);
			});
		auto all_other_locations_occupied = std::ranges::all_of(other_locations_with_only_the_industry, find_factory_at_location);
		if (not all_other_locations_occupied) {
			return m2::make_unexpected("Cannot build industry while the city has an empty location with only that industry");
		}
	}

	// Check if it's the first era and the player already has a tile in the city
	if (player.get_resource(m2g::pb::ERA) == 1.0f) {
		// Iterate over other locations in the city
		for (const auto& other_location : other_locations_in_city) {
			// If the location has a tile built
			if (auto* tile_obj = find_factory_at_location(other_location); tile_obj) {
				// If the parent of the tile is the given player
				if (tile_obj->parent_id() == player.parent().id()) {
					return m2::make_unexpected("Cannot build multiple industries in same city in first era");
				}
			}
		}
	}

	// Check if the player has a tile to build
	auto tile_type = get_next_buildable_factory(player, industry_card_to_tile_category(industry));
	if (not tile_type) {
		return m2::make_unexpected("Player doesn't have tiles left of selected industry");
	}
	const auto& tile = M2_GAME.get_named_item(*tile_type);

	// Check if the tile can be built in this era
	if (player.get_resource(m2g::pb::ERA) == tile.get_attribute(m2g::pb::FORBIDDEN_ERA)) {
		return m2::make_unexpected("The next tile of selected industry cannot be built in this era");
	}

	// Check if player has enough money
	if (player.get_resource(m2g::pb::MONEY) < tile.get_attribute(m2g::pb::MONEY_COST)) {
		return m2::make_unexpected("Player doesn't have enough money");
	}

	return *tile_type;
}
