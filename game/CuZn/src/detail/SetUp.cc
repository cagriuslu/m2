#include <cuzn/detail/SetUp.h>
#include <m2g_SpriteType.pb.h>
#include <m2/Error.h>
#include <m2/protobuf/Detail.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <random>

using namespace m2g;
using namespace m2g::pb;

std::vector<m2g::pb::SpriteType> PossiblyActiveMerchantLocations(int client_count) {
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
			throw M2_ERROR("Invalid client count");
	}
}

std::vector<m2g::pb::ItemType> PrepareMerchantLicenseList(int client_count) {
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
				throw M2_ERROR("Invalid client count");
		}
	}();

	// Prepare the list
	std::vector<m2g::pb::ItemType> merchant_licenses;
	M2_GAME.ForEachNamedItem([&merchant_licenses, count_attr](MAYBE m2g::pb::ItemType item_type, const m2::NamedItem& item) {
		if (item.category() == pb::ITEM_CATEGORY_MERCHANT_LICENSE) {
			auto license_count = m2::iround(item.get_attribute(count_attr));
			merchant_licenses.insert(merchant_licenses.end(), license_count, item.type());
		}
		return true;
	});

	// Shuffle the licenses
	std::random_device rd;
	std::mt19937 license_shuffler(rd());
	std::shuffle(merchant_licenses.begin(), merchant_licenses.end(), license_shuffler);

	return merchant_licenses;
}

std::vector<m2g::pb::ItemType> PrepareDrawDeck(int client_count) {
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
				throw M2_ERROR("Invalid client count");
		}
	}();

	// Prepare deck
	std::vector<m2g::pb::ItemType> draw_deck;
	M2_GAME.ForEachNamedItem([&draw_deck, count_attr](MAYBE m2g::pb::ItemType item_type, const m2::NamedItem& item) {
		if (item.category() == pb::ITEM_CATEGORY_INDUSTRY_CARD || item.category() == pb::ITEM_CATEGORY_CITY_CARD) {
			auto card_count = static_cast<int>(item.get_attribute(count_attr));
			draw_deck.insert(draw_deck.end(), card_count, item.type());
		}
		return true;
	});

	// Shuffle the cards
	std::random_device rd;
	std::mt19937 card_shuffler(rd());
	std::shuffle(draw_deck.begin(), draw_deck.end(), card_shuffler);

	return draw_deck;
}

void Give8CardsToEachPlayer(std::vector<ItemType>& deck) {
	for (int i = 0; i < M2G_PROXY.multiPlayerObjectIds.size(); ++i) {
		const auto playerObjectId = M2G_PROXY.multiPlayerObjectIds[i];
		m2_repeat(8) {
			// Draw card
			const auto cardType = deck.back();
			deck.pop_back();
			// Add card
			const auto& card = M2_GAME.GetNamedItem(cardType);
			M2_LEVEL.objects[playerObjectId].character().add_named_item(card);
			LOG_DEBUG("Giving card to player", i, card.in_game_name());
		}
	}
}
