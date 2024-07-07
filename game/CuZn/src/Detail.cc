#include <cuzn/Detail.h>
#include <m2/Game.h>
#include <random>

using namespace m2g;
using namespace m2g::pb;

bool is_card(Card card) {
	const auto& card_item = M2_GAME.get_named_item(card);
	return (card_item.category() == ITEM_CATEGORY_WILD_CARD ||
		card_item.category() == ITEM_CATEGORY_INDUSTRY_CARD ||
		card_item.category() == ITEM_CATEGORY_CITY_CARD);
}

bool is_city(City city) {
	return is_industry_city(city) || is_merchant_city(city);
}

bool is_industry_city(IndustryCity city) {
	return (BELPER_CARD <= city && city <= STANDALONE_LOCATION_2_CARD);
}
bool is_merchant_city(MerchantCity city) {
	return (GLOUCESTER_CARD <= city && city <= WARRINGTON_CARD);
}

bool is_industry(Industry industry) {
	return (COTTON_MILL_CARD <= industry && industry <= MANUFACTURED_GOODS_CARD);
}

bool is_industry_tile(IndustryTile industry_tile) {
	return (COTTON_MILL_TILEI <= industry_tile && industry_tile <= MANUFACTURED_GOODS_TILE_VIII);
}

bool is_industry_tile_level_1(IndustryTile industry_tile) {
	static std::set<IndustryTile> level_1_tiles{COTTON_MILL_TILEI, IRON_WORKS_TILE_I, BREWERY_TILE_I, COAL_MINE_TILE_I,
		POTTERY_TILEI, MANUFACTURED_GOODS_TILE_I};
	return level_1_tiles.contains(industry_tile);
}

bool is_industry_tile_category(IndustryTileCategory category) {
	return (ITEM_CATEGORY_COTTON_MILL_TILE <= category && category <= ITEM_CATEGORY_MANUFACTURED_GOODS_TILE);
}

bool is_industry_sprite(IndustrySprite industry_sprite) {
	switch (industry_sprite) {
		case SPRITE_BREWERY:
		case SPRITE_COAL_MINE:
		case SPRITE_COTTON_MILL:
		case SPRITE_IRON_WORKS:
		case SPRITE_MANUFACTURED_GOODS:
		case SPRITE_POTTERY:
			return true;
		default:
			return false;
	}
}

bool is_location(Location location) {
	return is_industry_location(location) || is_merchant_location(location);
}

bool is_industry_location(IndustryLocation location) {
	return (BELPER_COTTON_MILL_MANUFACTURED_GOODS <= location && location <= STANDALONE_BREWERY_2);
}

bool is_merchant_location(MerchantLocation location) {
	return GLOUCESTER_1 <= location && location <= WARRINGTON_2;
}

bool is_connection(Connection connection) {
	return is_canal(connection) || is_railroad(connection);
}

bool is_canal(Connection connection) {
	switch (connection) {
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

bool is_railroad(Connection connection) {
	switch (connection) {
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

bool is_canal_license(m2g::pb::ItemType item) {
	return item == m2g::pb::CANAL_LICENSE;
}

bool is_railroad_license(m2g::pb::ItemType item) {
	return item == m2g::pb::RAILROAD_LICENSE;
}

bool location_has_industry(IndustryLocation location, Industry industry) {
	auto industries = industries_on_location(location);
	return std::ranges::find(industries, industry) != industries.end();
}

City city_of_location(Location location) {
	if (not is_location(location)) {
		throw M2ERROR("Sprite is not a location");
	}

	for (const auto& named_item : M2_GAME.get_sprite(location).named_items()) {
		if (M2_GAME.get_named_item(named_item).category() == ITEM_CATEGORY_CITY_CARD) {
			return named_item;
		}
	}
	throw M2ERROR("Industry does not belong to a city");
}

Industry industry_of_industry_tile(IndustryTile industry_tile) {
	return industry_of_industry_tile_category(
		industry_tile_category_of_industry_tile(
			industry_tile));
}

Industry industry_of_industry_tile_category(IndustryTileCategory category) {
	switch (category) {
		case ITEM_CATEGORY_COTTON_MILL_TILE:
			return COTTON_MILL_CARD;
		case ITEM_CATEGORY_IRON_WORKS_TILE:
			return IRON_WORKS_CARD;
		case ITEM_CATEGORY_BREWERY_TILE:
			return BREWERY_CARD;
		case ITEM_CATEGORY_COAL_MINE_TILE:
			return COAL_MINE_CARD;
		case ITEM_CATEGORY_POTTERY_TILE:
			return POTTERY_CARD;
		case ITEM_CATEGORY_MANUFACTURED_GOODS_TILE:
			return MANUFACTURED_GOODS_CARD;
		default:
			throw M2ERROR("ItemCategory is not an industry item category");
	}
}

IndustryTileCategory industry_tile_category_of_industry(Industry industry_card) {
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

IndustryTileCategory industry_tile_category_of_industry_tile(IndustryTile industry_tile) {
	const auto& item = M2_GAME.get_named_item(industry_tile);
	return item.category();
}

IndustrySprite industry_sprite_of_industry(Industry industry) {
	switch (industry) {
		case COTTON_MILL_CARD:
			return SPRITE_COTTON_MILL;
		case IRON_WORKS_CARD:
			return SPRITE_IRON_WORKS;
		case BREWERY_CARD:
			return SPRITE_BREWERY;
		case COAL_MINE_CARD:
			return SPRITE_COAL_MINE;
		case POTTERY_CARD:
			return SPRITE_POTTERY;
		case MANUFACTURED_GOODS_CARD:
			return SPRITE_MANUFACTURED_GOODS;
		default:
			throw M2ERROR("Invalid industry card");
	}
}

MerchantLocation merchant_location_of_merchant_city(MerchantCity city) {
	if (not is_merchant_city(city)) {
		throw M2ERROR("Invalid merchant city");
	}

	std::optional<MerchantLocation> location;
	M2_GAME.for_each_sprite([&location, city](m2g::pb::SpriteType sprite_type, const m2::Sprite& sprite) {
		if (is_merchant_location(sprite_type)) {
			auto it = std::ranges::find(sprite.named_items(), city);
			if (it != sprite.named_items().end()) {
				location = sprite_type;
				return false;
			}
		}
		return true;
	});
	if (not location) {
		throw M2ERROR("Unable to find location of merchant city");
	}
	return *location;
}

std::vector<IndustryLocation> industry_locations_in_city(City city_card) {
	if (not is_city(city_card)) {
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

std::vector<Connection> connections_from_city(City city_card) {
	std::vector<Connection> connections;
	M2_GAME.for_each_sprite([&connections, city_card](m2g::pb::SpriteType sprite_type, const m2::Sprite& sprite) {
		auto city_license = std::ranges::find(sprite.named_items(), city_card);
		if (city_license != sprite.named_items().end()) {
			auto canal_license = std::ranges::find(sprite.named_items(), CANAL_LICENSE);
			auto railroad_license = std::ranges::find(sprite.named_items(), RAILROAD_LICENSE);
			if (canal_license != sprite.named_items().end() || railroad_license != sprite.named_items().end()) {
				connections.emplace_back(sprite_type);
			}
		}
		return true;
	});
	return connections;
}

std::vector<City> cities_from_connection(Connection connection) {
	std::vector<City> cities;
	auto connection_items = M2_GAME.get_sprite(connection).named_items();
	for (auto item_type : connection_items) {
		if (M2_GAME.get_named_item(item_type).category() == ITEM_CATEGORY_CITY_CARD) {
			cities.emplace_back(item_type);
		}
	}
	return cities;
}

std::vector<Industry> industries_on_location(IndustryLocation location) {
	if (not is_industry_location(location)) {
		throw M2ERROR("Sprite is not an industry location");
	}

	std::vector<m2g::pb::ItemType> industries;
	for (const auto& named_item : M2_GAME.get_sprite(location).named_items()) {
		if (M2_GAME.get_named_item(named_item).category() == ITEM_CATEGORY_INDUSTRY_CARD) {
			industries.emplace_back(named_item);
		}
	}
	return industries;
}

m2::VecF connection_sprite_world_offset(m2g::pb::SpriteType original_type) {
	if (original_type == pb::CANAL_OR_RAILROAD_BACKGROUND_1) {
		return {};
	} else if (original_type == pb::CANAL_OR_RAILROAD_BACKGROUND_2) {
		return {0.5f, 0.0f};
	} else if (original_type == pb::CANAL_OR_RAILROAD_BACKGROUND_3) {
		return {0.0f, 0.5f};
	} else if (original_type == pb::CANAL_OR_RAILROAD_BACKGROUND_4) {
		return {0.5f, 0.5f};
	} else {
		throw M2ERROR("Invalid connection sprite");
	}
}

std::optional<IndustryLocation> industry_location_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(M2G_PROXY.industry_positions.begin(), M2G_PROXY.industry_positions.end(),
			[&](const auto& pos_and_type) { return pos_and_type.second.second.contains(world_position); });
	if (it != M2G_PROXY.industry_positions.end()) {
		return it->first;
	} else {
		return std::nullopt;
	}
}

std::optional<MerchantLocation> merchant_location_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(M2G_PROXY.merchant_positions.begin(), M2G_PROXY.merchant_positions.end(),
		[&](const auto& pos_and_type) { return pos_and_type.second.second.contains(world_position); });
	if (it != M2G_PROXY.merchant_positions.end()) {
		return it->first;
	}
	return std::nullopt;
}

m2::VecF position_of_industry_location(IndustryLocation industry_location) {
	if (not is_industry_location(industry_location)) {
		throw M2ERROR("Invalid industry location");
	}
	if (auto it = M2G_PROXY.industry_positions.find(industry_location); it != M2G_PROXY.industry_positions.end()) {
		return it->second.first;
	} else {
		throw M2ERROR("Industry location not found in position map");
	}
}

std::optional<Connection> connection_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(M2G_PROXY.connection_positions.begin(), M2G_PROXY.connection_positions.end(),
		[&](const auto& pos_and_type) { return pos_and_type.second.second.contains(world_position); });
	if (it != M2G_PROXY.connection_positions.end()) {
		return it->first;
	} else {
		return std::nullopt;
	}
}

m2::VecF position_of_connection(Connection connection) {
	if (not is_connection(connection)) {
		throw M2ERROR("Invalid connection");
	}
	if (auto it = M2G_PROXY.connection_positions.find(connection); it != M2G_PROXY.connection_positions.end()) {
		return it->second.first;
	} else {
		throw M2ERROR("Connection not found in position map");
	}
}

namespace {
	std::vector<int> income_point_level_points = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
												  3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
												  4, 4, 4, 4, 4, 4, 4, 4, 4, 3};
}

int income_level_from_income_points(int ip) {
	if (ip < -10 || 89 < ip) {
		throw M2ERROR("Invalid income points");
	}

	if (ip <= 0) {
		return ip;
	} else {
		for (size_t level_minus_one = 0; level_minus_one < income_point_level_points.size(); ++level_minus_one) {
			ip -= income_point_level_points[level_minus_one];
			if (ip <= 0) {
				return static_cast<int>(level_minus_one) + 1;
			}
		}
		throw M2ERROR("Invalid income_point_level_points map");
	}
}

int highest_income_points_of_level(int level) {
	if (level < -10 || 30 < level) {
		throw M2ERROR("Invalid income level");
	}

	if (level <= 0) {
		return level;
	} else {
		int ip = 0;
		for (int i = 0; i < level; ++i) {
			ip += income_point_level_points[i];
		}
		return ip;
	}
}
