#include <cuzn/Detail.h>
#include <m2/Game.h>

using namespace m2g;
using namespace m2g::pb;

bool cuzn::is_card(m2g::pb::ItemType item) {
	const auto& card_item = GAME.get_named_item(item);
	return (card_item.category() == ITEM_CATEGORY_WILD_CARD ||
		card_item.category() == ITEM_CATEGORY_INDUSTRY_CARD ||
		card_item.category() == ITEM_CATEGORY_CITY_CARD);
}

bool cuzn::is_industry_location(m2g::pb::SpriteType sprite) {
	return (BELPER_COTTON_MILL_MANUFACTURED_GOODS <= sprite && sprite <= STANDALONE_BREWERY_2);
}

m2g::pb::ItemType cuzn::city_of_location(m2g::pb::SpriteType location) {
	if (not is_industry_location(location)) {
		throw M2ERROR("Sprite is not an industry location");
	}

	for (const auto& named_item : GAME.get_sprite(location).named_items()) {
		if (GAME.get_named_item(named_item).category() == ITEM_CATEGORY_CITY_CARD) {
			return named_item;
		}
	}
	throw M2ERROR("Industry does not belong to a city");
}

std::vector<m2g::pb::SpriteType> cuzn::locations_in_city(m2g::pb::ItemType city_card) {
	if (GAME.get_named_item(city_card).category() != ITEM_CATEGORY_CITY_CARD) {
		throw M2ERROR("Card does not belong to a city");
	}

	std::vector<m2g::pb::SpriteType> industry_locations;
	// Iterate over industries
	for (auto industry_location = BELPER_COTTON_MILL_MANUFACTURED_GOODS; industry_location <= STANDALONE_BREWERY_2;
		industry_location = static_cast<m2g::pb::SpriteType>(m2::I(industry_location) + 1)) {
		if (city_of_location(industry_location) == city_card) {
			industry_locations.emplace_back(industry_location);
		}
	}
	return industry_locations;
}

std::vector<m2g::pb::ItemType> cuzn::industries_on_location(m2g::pb::SpriteType location) {
	if (not is_industry_location(location)) {
		throw M2ERROR("Sprite is not an industry location");
	}

	std::vector<m2g::pb::ItemType> industries;
	for (const auto& named_item : GAME.get_sprite(location).named_items()) {
		if (GAME.get_named_item(named_item).category() == ITEM_CATEGORY_INDUSTRY_CARD) {
			industries.emplace_back(named_item);
		}
	}
	return industries;
}

std::vector<m2g::pb::ItemType> cuzn::selectable_industries(m2g::pb::ItemType selected_card, m2g::pb::SpriteType selected_location) {
	if (not is_card(selected_card)) {
		throw M2ERROR("Item is not a card");
	}
	if (not is_industry_location(selected_location)) {
		throw M2ERROR("Sprite is not an industry location");
	}

	const auto& selected_card_item = GAME.get_named_item(selected_card);
	const auto& selected_sprite_sprite = GAME.get_sprite(selected_location);
	// Lookup industries on the sprite
	std::vector<m2g::pb::ItemType> selected_sprite_industries;
	std::copy_if(selected_sprite_sprite.named_items().begin(), selected_sprite_sprite.named_items().end(), std::back_inserter(selected_sprite_industries), [](auto item_type) {
		return (GAME.get_named_item(item_type).category() == ITEM_CATEGORY_INDUSTRY_CARD);
	});
	if (selected_sprite_industries.empty()) {
		throw M2ERROR("Selected sprite does not hold any industry cards");
	}
	// Look up the location of the sprite
	auto location_card_it = std::find_if(selected_sprite_sprite.named_items().begin(), selected_sprite_sprite.named_items().end(), [](auto item_type) {
		return (GAME.get_named_item(item_type).category() == ITEM_CATEGORY_CITY_CARD);
	});
	if (location_card_it == selected_sprite_sprite.named_items().end()) {
		throw M2ERROR("Selected sprite does not hold a location card");
	}
	m2g::pb::ItemType selected_sprite_location = *location_card_it;

	// If the card is wild card
	if (selected_card_item.category() == ITEM_CATEGORY_WILD_CARD) {
		// Any industry in the selected location can be built
		return selected_sprite_industries;
	} else if (selected_card_item.category() == ITEM_CATEGORY_INDUSTRY_CARD) {
		// Check if the selected industry exists in the sprite's industries
		auto industry_card_it = std::find(selected_sprite_industries.begin(), selected_sprite_industries.end(), selected_card);
		if (industry_card_it == selected_sprite_industries.end()) {
			return {}; // No selectable industries
		} else {
			return {selected_card}; // Only the selected industry card is selectable
		}
	} else { // ITEM_CATEGORY_CITY_CARD
		// Check if the card belongs to this location
		if (selected_card == selected_sprite_location) {
			// Any industry in the selected location can be built
			return selected_sprite_industries;
		} else {
			return {}; // No selectable industries
		}
	}
}

m2g::pb::ItemCategory cuzn::industry_card_to_tile_category(m2g::pb::ItemType industry_card) {
	switch (industry_card) {
		case COTTON_MILL_CARD:
			return ITEM_CATEGORY_COTTON_MILL_TILE;
		case IRON_WORKS_CARD:
			return ITEM_CATEGORY_IRON_WORKS_TILE;
		case BREWERY_CARD:
			return ITEM_CATEGORY_BREWERY_TILE;
		case COAL_MINE_CARD:
			return ITEM_CATEGORY_COAL_MINE_TILE;
		case POTTERY_CARD:
			return ITEM_CATEGORY_POTTERY_TILE;
		case MANUFACTURED_GOODS_CARD:
			return ITEM_CATEGORY_MANUFACTURED_GOODS_TILE;
		default:
			throw M2ERROR("Item is not an industry card");
	}
}
