#include <cuzn/detail/SetUp.h>
#include <m2g_SpriteType.pb.h>
#include <m2/Exception.h>
#include <m2/protobuf/Detail.h>
#include <m2/Game.h>
#include <random>

using namespace m2g;
using namespace m2g::pb;

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
