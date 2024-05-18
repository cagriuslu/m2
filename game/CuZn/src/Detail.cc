#include <cuzn/Detail.h>
#include <m2/Game.h>
#include <random>

using namespace m2g;
using namespace m2g::pb;

bool cuzn::is_card(Card card) {
	const auto& card_item = M2_GAME.get_named_item(card);
	return (card_item.category() == ITEM_CATEGORY_WILD_CARD ||
		card_item.category() == ITEM_CATEGORY_INDUSTRY_CARD ||
		card_item.category() == ITEM_CATEGORY_CITY_CARD);
}

bool cuzn::is_city(City city) {
	return (BELPER_CARD <= city && city <= STANDALONE_LOCATION_2_CARD);
}

bool cuzn::is_industry(Industry industry) {
	return (COTTON_MILL_CARD <= industry && industry <= MANUFACTURED_GOODS_CARD);
}

bool cuzn::is_industry_tile(IndustryTile industry_tile) {
	return (COTTON_MILL_TILEI <= industry_tile && industry_tile <= MANUFACTURED_GOODS_TILE_VIII);
}

bool cuzn::is_industry_tile_category(IndustryTileCategory category) {
	return (ITEM_CATEGORY_COTTON_MILL_TILE <= category && category <= ITEM_CATEGORY_MANUFACTURED_GOODS_TILE);
}

bool cuzn::is_industry_sprite(IndustrySprite industry_sprite) {
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

bool cuzn::is_canal_license(m2g::pb::ItemType item) {
	return item == m2g::pb::CANAL_LICENSE;
}

bool cuzn::is_railroad_license(m2g::pb::ItemType item) {
	return item == m2g::pb::RAILROAD_LICENSE;
}

bool cuzn::is_canal_era() {
	return M2_PLAYER.character().get_resource(ERA) == 1.0f;
}

bool cuzn::is_railroad_era() {
	return M2_PLAYER.character().get_resource(ERA) == 2.0f;
}

cuzn::City cuzn::city_of_industry_location(IndustryLocation location) {
	if (not is_industry_location(location)) {
		throw M2ERROR("Sprite is not an industry location");
	}

	for (const auto& named_item : M2_GAME.get_sprite(location).named_items()) {
		if (M2_GAME.get_named_item(named_item).category() == ITEM_CATEGORY_CITY_CARD) {
			return named_item;
		}
	}
	throw M2ERROR("Industry does not belong to a city");
}

cuzn::Industry cuzn::industry_of_industry_tile(IndustryTile industry_tile) {
	return industry_of_industry_tile_category(
		industry_tile_category_of_industry_tile(
			industry_tile));
}

cuzn::Industry cuzn::industry_of_industry_tile_category(cuzn::IndustryTileCategory category) {
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

cuzn::IndustryTileCategory cuzn::industry_tile_category_of_industry(Industry industry_card) {
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

cuzn::IndustryTileCategory cuzn::industry_tile_category_of_industry_tile(IndustryTile industry_tile) {
	const auto& item = M2_GAME.get_named_item(industry_tile);
	return item.category();
}

cuzn::IndustrySprite cuzn::industry_sprite_of_industry(Industry industry) {
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

std::vector<cuzn::IndustryLocation> cuzn::locations_in_city(City city_card) {
	if (not is_city(city_card)) {
		throw M2ERROR("Card does not belong to a city");
	}

	std::vector<m2g::pb::SpriteType> industry_locations;
	// Iterate over industries
	for (auto industry_location = BELPER_COTTON_MILL_MANUFACTURED_GOODS; industry_location <= STANDALONE_BREWERY_2;
		industry_location = static_cast<m2g::pb::SpriteType>(m2::I(industry_location) + 1)) {
		if (city_of_industry_location(industry_location) == city_card) {
			industry_locations.emplace_back(industry_location);
		}
	}
	return industry_locations;
}

std::vector<cuzn::Industry> cuzn::industries_on_location(IndustryLocation location) {
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

bool cuzn::location_has_industry(IndustryLocation location, Industry industry) {
	auto industries = industries_on_location(location);
	return std::ranges::find(industries, industry) != industries.end();
}

std::optional<cuzn::IndustryLocation> cuzn::industry_location_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(M2G_PROXY.industry_positions.begin(), M2G_PROXY.industry_positions.end(),
			[&](const auto& pos_and_type) { return pos_and_type.second.second.contains(world_position); });
	if (it != M2G_PROXY.industry_positions.end()) {
		return it->first;
	} else {
		return std::nullopt;
	}
}

m2::VecF cuzn::position_of_industry_location(IndustryLocation industry_location) {
	if (not is_industry_location(industry_location)) {
		throw M2ERROR("Invalid industry location");
	}
	if (auto it = M2G_PROXY.industry_positions.find(industry_location); it != M2G_PROXY.industry_positions.end()) {
		return it->second.first;
	} else {
		throw M2ERROR("Industry location not found in position map");
	}
}

std::optional<cuzn::NetworkLocation> cuzn::network_location_on_position(const m2::VecF& world_position) {
	auto it = std::find_if(M2G_PROXY.network_positions.begin(), M2G_PROXY.network_positions.end(),
		[&](const auto& pos_and_type) { return pos_and_type.second.contains(world_position); });
	if (it != M2G_PROXY.network_positions.end()) {
		return it->first;
	} else {
		return std::nullopt;
	}
}

SDL_Color cuzn::generate_player_color(unsigned index) {
	switch (index) {
		case 0:
			return SDL_Color{165, 42, 42, 255};
		case 1:
			return SDL_Color{173, 255, 47, 255};
		case 2:
			return SDL_Color{32, 178, 170, 255};
		case 3:
			return SDL_Color{255, 165, 0, 255};
		default:
			throw M2ERROR("Invalid player index");
	}
}

std::vector<m2g::pb::SpriteType> cuzn::active_merchant_locations(int client_count) {
	switch (client_count) {
		case 2:
			return {pb::GLOUCESTER_1, pb::GLOUCESTER_2, pb::SHREWSBURY_1, pb::OXFORD_1, pb::OXFORD_2};
		case 3:
			return {pb::GLOUCESTER_1, pb::GLOUCESTER_2, pb::SHREWSBURY_1, pb::OXFORD_1,
					pb::OXFORD_2,     pb::WARRINGTON_1, pb::WARRINGTON_2};
		case 4:
			return {pb::GLOUCESTER_1, pb::GLOUCESTER_2, pb::SHREWSBURY_1, pb::OXFORD_1,    pb::OXFORD_2,
					pb::NOTTINGHAM_1, pb::NOTTINGHAM_2, pb::WARRINGTON_1, pb::WARRINGTON_2};
		default:
			throw M2ERROR("Invalid client count");
	}
}

std::vector<m2g::pb::ItemType> cuzn::prepare_merchant_license_list(int client_count) {
	// Figure out the attribute to use for card selection
	m2g::pb::AttributeType count_attr = [=]() {
		switch (client_count) {
			case 2:
				return m2g::pb::MERCHANT_COUNT_IN_2_PLAYER_GAME;
			case 3:
				return m2g::pb::MERCHANT_COUNT_IN_3_PLAYER_GAME;
			case 4:
				return m2g::pb::MERCHANT_COUNT_IN_4_PLAYER_GAME;
			default:
				throw M2ERROR("Invalid client count");
		}
	}();

	// Prepare the list
	std::vector<m2g::pb::ItemType> merchant_license_list;
	for (auto i = 0; i < m2::pb::enum_value_count<m2g::pb::ItemType>(); ++i) {
		auto item_type = m2::pb::enum_value<m2g::pb::ItemType>(i);
		const auto& item = M2_GAME.get_named_item(item_type);
		if (item.category() == pb::ITEM_CATEGORY_MERCHANT_LICENSE) {
			auto license_count = static_cast<int>(item.get_attribute(count_attr));
			merchant_license_list.insert(merchant_license_list.end(), license_count, item.type());
		}
	}

	// Shuffle the licenses
	std::random_device rd;
	std::mt19937 license_shuffler(rd());
	std::shuffle(merchant_license_list.begin(), merchant_license_list.end(), license_shuffler);

	return merchant_license_list;
}

std::vector<m2g::pb::ItemType> cuzn::prepare_draw_deck(int client_count) {
	// Figure out the attribute to use for card selection
	m2g::pb::AttributeType count_attr = [=]() {
		switch (client_count) {
			case 2:
				return m2g::pb::COUNT_IN_2_PLAYER_GAME;
			case 3:
				return m2g::pb::COUNT_IN_3_PLAYER_GAME;
			case 4:
				return m2g::pb::COUNT_IN_4_PLAYER_GAME;
			default:
				throw M2ERROR("Invalid client count");
		}
	}();

	// Prepare deck
	std::vector<m2g::pb::ItemType> draw_deck;
	for (auto i = 0; i < m2::pb::enum_value_count<m2g::pb::ItemType>(); ++i) {
		auto item_type = m2::pb::enum_value<m2g::pb::ItemType>(i);
		const auto& item = M2_GAME.get_named_item(item_type);
		if (item.category() == pb::ITEM_CATEGORY_INDUSTRY_CARD || item.category() == pb::ITEM_CATEGORY_CITY_CARD) {
			auto card_count = static_cast<int>(item.get_attribute(count_attr));
			draw_deck.insert(draw_deck.end(), card_count, item.type());
		}
	}

	// Shuffle the cards
	std::random_device rd;
	std::mt19937 card_shuffler(rd());
	std::shuffle(draw_deck.begin(), draw_deck.end(), card_shuffler);

	return draw_deck;
}
