#include <cuzn/Detail.h>
#include <m2/Game.h>

using namespace m2g;
using namespace m2g::pb;

bool cuzn::is_card(Card card) {
	const auto& card_item = GAME.get_named_item(card);
	return (card_item.category() == ITEM_CATEGORY_WILD_CARD ||
		card_item.category() == ITEM_CATEGORY_INDUSTRY_CARD ||
		card_item.category() == ITEM_CATEGORY_CITY_CARD);
}

bool cuzn::is_industry(Industry industry) {
	return (COTTON_MILL_CARD <= industry && industry <= MANUFACTURED_GOODS_CARD);
}

bool cuzn::is_industry_location(IndustryLocation location) {
	return (BELPER_COTTON_MILL_MANUFACTURED_GOODS <= location && location <= STANDALONE_BREWERY_2);
}

bool cuzn::is_canal_location(NetworkLocation location) {
	switch (location) {
		case BELPER_DERBY_CANAL_RAILROAD:
		case DERBY_BURTON_UPON_TRENT_CANAL_RAILROAD:
		case DERBY_NOTTINGHAM_CANAL_RAILROAD:
		case LEEK_STOKE_ON_TRENT_CANAL_RAILROAD:
		case STOKE_ON_TRENT_STONE_CANAL_RAILROAD:
		case STOKE_ON_TRENT_WARRINGTON_CANAL_RAILROAD:
		case STONE_STAFFORD_CANAL_RAILROAD:
		case STONE_BURTON_UPON_TRENT_CANAL_RAILROAD:
		case STAFFORD_CANNOCK_CANAL_RAILROAD:
		case BURTON_UPON_TRENT_TAMWORTH_CANAL_RAILROAD:
		case BURTON_UPON_TRENT_WALSALL_CANAL:
		case CANNOCK_BREWERY1_CANAL_RAILROAD:
		case CANNOCK_WALSALL_CANAL_RAILROAD:
		case CANNOCK_WOLFERHAMPTON_CANAL_RAILROAD:
		case TAMWORTH_BIRMINGHAM_CANAL_RAILROAD:
		case TAMWORTH_NUNEATON_CANAL_RAILROAD:
		case WALSALL_WOLFERHAMPTON_CANAL_RAILROAD:
		case WALSALL_BIRMINGHAM_CANAL_RAILROAD:
		case COALBROOKEDALE_KIDDERMINSTER_CANAL_RAILROAD:
		case COALBROOKEDALE_WOLFERHAMPTON_CANAL_RAILROAD:
		case COALBROOKEDALE_SHREWSBURY_CANAL_RAILROAD:
		case DUDLEY_KIDDERMINSTER_CANAL_RAILROAD:
		case DUDLEY_WOLFERHAMPTON_CANAL_RAILROAD:
		case DUDLEY_BIRMINGHAM_CANAL_RAILROAD:
		case KIDDERMINSTER_WORCESTER_BREWERY2_CANAL_RAILROAD:
		case WORCESTER_BIRMINGHAM_CANAL_RAILROAD:
		case WORCESTER_GLOUCESTER_CANAL_RAILROAD:
		case BIRMINGHAM_COVENTRY_CANAL_RAILROAD:
		case BIRMINGHAM_OXFORD_CANAL_RAILROAD:
		case REDDITCH_GLOUCESTER_CANAL_RAILROAD:
		case REDDITCH_OXFORD_CANAL_RAILROAD:
			return true;
		default:
			return false;
	}
}

bool cuzn::is_railroad_location(NetworkLocation location) {
	switch (location) {
		case BELPER_DERBY_CANAL_RAILROAD:
		case BELPER_LEEK_RAILROAD:
		case DERBY_UTTOXETER_RAILROAD:
		case DERBY_BURTON_UPON_TRENT_CANAL_RAILROAD:
		case DERBY_NOTTINGHAM_CANAL_RAILROAD:
		case LEEK_STOKE_ON_TRENT_CANAL_RAILROAD:
		case STOKE_ON_TRENT_STONE_CANAL_RAILROAD:
		case STOKE_ON_TRENT_WARRINGTON_CANAL_RAILROAD:
		case STONE_UTTOXETER_RAILROAD:
		case STONE_STAFFORD_CANAL_RAILROAD:
		case STONE_BURTON_UPON_TRENT_CANAL_RAILROAD:
		case STAFFORD_CANNOCK_CANAL_RAILROAD:
		case BURTON_UPON_TRENT_CANNOCK_RAILROAD:
		case BURTON_UPON_TRENT_TAMWORTH_CANAL_RAILROAD:
		case CANNOCK_BREWERY1_CANAL_RAILROAD:
		case CANNOCK_WALSALL_CANAL_RAILROAD:
		case CANNOCK_WOLFERHAMPTON_CANAL_RAILROAD:
		case TAMWORTH_WALSALL_RAILROAD:
		case TAMWORTH_BIRMINGHAM_CANAL_RAILROAD:
		case TAMWORTH_NUNEATON_CANAL_RAILROAD:
		case WALSALL_WOLFERHAMPTON_CANAL_RAILROAD:
		case WALSALL_BIRMINGHAM_CANAL_RAILROAD:
		case COALBROOKEDALE_KIDDERMINSTER_CANAL_RAILROAD:
		case COALBROOKEDALE_WOLFERHAMPTON_CANAL_RAILROAD:
		case COALBROOKEDALE_SHREWSBURY_CANAL_RAILROAD:
		case DUDLEY_KIDDERMINSTER_CANAL_RAILROAD:
		case DUDLEY_WOLFERHAMPTON_CANAL_RAILROAD:
		case DUDLEY_BIRMINGHAM_CANAL_RAILROAD:
		case KIDDERMINSTER_WORCESTER_BREWERY2_CANAL_RAILROAD:
		case WORCESTER_BIRMINGHAM_CANAL_RAILROAD:
		case WORCESTER_GLOUCESTER_CANAL_RAILROAD:
		case BIRMINGHAM_COVENTRY_CANAL_RAILROAD:
		case BIRMINGHAM_NUNEATON_RAILROAD:
		case BIRMINGHAM_REDDITCH_RAILROAD:
		case BIRMINGHAM_OXFORD_CANAL_RAILROAD:
		case COVENTRY_NUNEATON_RAILROAD:
		case REDDITCH_GLOUCESTER_CANAL_RAILROAD:
		case REDDITCH_OXFORD_CANAL_RAILROAD:
			return true;
		default:
			return false;
	}
}

bool cuzn::player_has_card(m2::Character& player, m2g::pb::ItemType card) {
	return player.find_items(card) != player.end_items();
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

bool cuzn::location_has_industry(SpriteType location, ItemType industry) {
	auto industries = industries_on_location(location);
	return std::ranges::find(industries, industry) != industries.end();
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

std::optional<m2g::pb::SpriteType> cuzn::industry_location_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(PROXY.industry_positions.begin(), PROXY.industry_positions.end(),
			[&](const auto& pos_and_type) { return pos_and_type.second.point_in_rect(world_position); });
	if (it != PROXY.industry_positions.end()) {
		return it->first;
	} else {
		return std::nullopt;
	}
}

std::optional<m2g::pb::SpriteType> cuzn::network_location_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(PROXY.network_positions.begin(), PROXY.network_positions.end(),
		[&](const auto& pos_and_type) { return pos_and_type.second.point_in_rect(world_position); });
	if (it != PROXY.network_positions.end()) {
		return it->first;
	} else {
		return std::nullopt;
	}
}
