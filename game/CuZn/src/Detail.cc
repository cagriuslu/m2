#include <cuzn/Detail.h>
#include <m2/Game.h>
#include <random>

using namespace m2g;
using namespace m2g::pb;

bool is_card(Card card) {
	const auto& card_item = M2_GAME.GetNamedItem(card);
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

bool is_sellable_industry(Industry industry) {
	return industry == COTTON_MILL_CARD || industry == POTTERY_CARD || industry == MANUFACTURED_GOODS_CARD;
}

bool is_industry_tile(IndustryTile industry_tile) {
	return (COTTON_MILL_TILE_I <= industry_tile && industry_tile <= MANUFACTURED_GOODS_TILE_VIII);
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

bool is_merchant_benefit_develop(MerchantLocation location) {
	auto merchant_city = city_of_location(location);
	return M2_GAME.GetNamedItem(merchant_city).has_attribute(MERCHANT_BONUS_DEVELOP);
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

bool is_poi(POI poi) {
	return is_location(poi) || is_connection(poi);
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
		throw M2_ERROR("Sprite is not a location");
	}

	for (const auto& named_item : M2_GAME.GetSprite(location).named_items()) {
		if (M2_GAME.GetNamedItem(named_item).category() == ITEM_CATEGORY_CITY_CARD) {
			return named_item;
		}
	}
	throw M2_ERROR("Industry does not belong to a city");
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
			throw M2_ERROR("ItemCategory is not an industry item category");
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
			throw M2_ERROR("Item is not an industry card");
	}
}

IndustryTileCategory industry_tile_category_of_industry_tile(IndustryTile industry_tile) {
	const auto& item = M2_GAME.GetNamedItem(industry_tile);
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
			throw M2_ERROR("Invalid industry card");
	}
}

m2g::pb::ItemType merchant_license_type_of_sellable_industry(SellableIndustry sellable_industry) {
	switch (sellable_industry) {
		case m2g::pb::COTTON_MILL_CARD:
			return COTTON_MILL_MERCHANT_LICENSE;
		case m2g::pb::POTTERY_CARD:
			return POTTERY_MERCHANT_LICENSE;
		case m2g::pb::MANUFACTURED_GOODS_CARD:
			return MANUFACTURED_GOODS_MERCHANT_LICENSE;
		default:
			throw M2_ERROR("Invalid sellable industry");
	}
}

std::vector<MerchantLocation> merchant_locations_of_merchant_city(MerchantCity city) {
	if (not is_merchant_city(city)) {
		throw M2_ERROR("Invalid merchant city");
	}

	std::vector<MerchantLocation> locations;
	M2_GAME.ForEachSprite([&locations, city](m2g::pb::SpriteType sprite_type, const m2::Sprite& sprite) {
		if (is_merchant_location(sprite_type)) {
			if (std::ranges::find(sprite.named_items(), city) != sprite.named_items().end()) {
				locations.emplace_back(sprite_type);
			}
		}
		return true;
	});
	if (locations.empty()) {
		throw M2_ERROR("Unable to find location of merchant city");
	}
	return locations;
}

std::set<IndustryLocation> all_industry_locations() {
	std::set<IndustryLocation> industry_locations;
	for (auto location = BELPER_COTTON_MILL_MANUFACTURED_GOODS;
		location <= STANDALONE_BREWERY_2;
		location = static_cast<IndustryLocation>(m2::I(location) + 1)) {
		industry_locations.insert(location);
	}
	return industry_locations;
}

std::set<Connection> all_canals() {
	std::set<IndustryLocation> connections;
	for (int i = m2g::pb::BELPER_DERBY_CANAL_RAILROAD; i <= m2g::pb::REDDITCH_OXFORD_CANAL_RAILROAD; ++i) {
		auto connection = static_cast<Connection>(i);
		if (is_canal(connection)) {
			connections.insert(connection);
		}
	}
	return connections;
}

std::set<Connection> all_railroads() {
	std::set<IndustryLocation> connections;
	for (int i = m2g::pb::BELPER_DERBY_CANAL_RAILROAD; i <= m2g::pb::REDDITCH_OXFORD_CANAL_RAILROAD; ++i) {
		auto connection = static_cast<Connection>(i);
		if (is_railroad(connection)) {
			connections.insert(connection);
		}
	}
	return connections;
}

std::vector<IndustryLocation> industry_locations_in_city(City city_card) {
	if (not is_city(city_card)) {
		throw M2_ERROR("Card does not belong to a city");
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
	M2_GAME.ForEachSprite([&connections, city_card](m2g::pb::SpriteType sprite_type, const m2::Sprite& sprite) {
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
	auto connection_items = M2_GAME.GetSprite(connection).named_items();
	for (auto item_type : connection_items) {
		if (M2_GAME.GetNamedItem(item_type).category() == ITEM_CATEGORY_CITY_CARD) {
			cities.emplace_back(item_type);
		}
	}
	return cities;
}

std::array<City, 2> major_cities_from_connection(Connection connection) {
	auto cities = cities_from_connection(connection);

	cities.erase(std::remove(cities.begin(), cities.end(), STANDALONE_LOCATION_2_CARD), cities.end());
	if (cities.size() != 2) {
		throw M2_ERROR("Invalid connection");
	}

	return {cities[0], cities[1]};
}

std::vector<Industry> industries_on_location(IndustryLocation location) {
	if (not is_industry_location(location)) {
		throw M2_ERROR("Sprite is not an industry location");
	}

	std::vector<m2g::pb::ItemType> industries;
	for (const auto& named_item : M2_GAME.GetSprite(location).named_items()) {
		if (M2_GAME.GetNamedItem(named_item).category() == ITEM_CATEGORY_INDUSTRY_CARD) {
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
		throw M2_ERROR("Invalid connection sprite");
	}
}

std::optional<IndustryLocation> industry_location_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(M2G_PROXY.industry_positions.begin(), M2G_PROXY.industry_positions.end(),
			[&](const auto& pos_and_type) { return std::get<m2::RectF>(pos_and_type.second).contains(world_position); });
	if (it != M2G_PROXY.industry_positions.end()) {
		return it->first;
	} else {
		return std::nullopt;
	}
}

std::optional<MerchantLocation> merchant_location_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(M2G_PROXY.merchant_positions.begin(), M2G_PROXY.merchant_positions.end(),
		[&](const auto& pos_and_type) { return std::get<m2::RectF>(pos_and_type.second).contains(world_position); });
	if (it != M2G_PROXY.merchant_positions.end()) {
		return it->first;
	}
	return std::nullopt;
}

m2::VecF position_of_industry_location(IndustryLocation industry_location) {
	if (not is_industry_location(industry_location)) {
		throw M2_ERROR("Invalid industry location");
	}
	if (auto it = M2G_PROXY.industry_positions.find(industry_location); it != M2G_PROXY.industry_positions.end()) {
		return std::get<m2::VecF>(it->second);
	} else {
		throw M2_ERROR("Industry location not found in position map");
	}
}

std::optional<Connection> connection_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(M2G_PROXY.connection_positions.begin(), M2G_PROXY.connection_positions.end(),
		[&](const auto& pos_and_type) { return std::get<m2::RectF>(pos_and_type.second).contains(world_position); });
	if (it != M2G_PROXY.connection_positions.end()) {
		return it->first;
	} else {
		return std::nullopt;
	}
}

m2::VecF position_of_connection(Connection connection) {
	if (not is_connection(connection)) {
		throw M2_ERROR("Invalid connection");
	}
	if (auto it = M2G_PROXY.connection_positions.find(connection); it != M2G_PROXY.connection_positions.end()) {
		return std::get<m2::VecF>(it->second);
	} else {
		throw M2_ERROR("Connection not found in position map");
	}
}
