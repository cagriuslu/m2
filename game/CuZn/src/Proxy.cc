#include <m2/Proxy.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Merchant.h>
#include <cuzn/object/Market.h>
#include <m2/Game.h>
#include <m2/multi_player/State.h>
#include <random>

void m2g::Proxy::post_multi_player_level_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	auto client_count = GAME.is_server() ? GAME.server_thread().client_count() : GAME.client_thread().total_player_count();

	// Add human players
	for (auto i = 0; i < client_count; ++i) {
		auto [client_obj, client_id] = m2::create_object(m2::VecF{i, i}, m2g::pb::ObjectType::HUMAN_PLAYER);
		auto client_init_result = cuzn::init_human_player(client_obj);
		m2_succeed_or_throw_error(client_init_result);
		PROXY.multi_player_object_ids.emplace_back(client_id);

		if (GAME.is_server()) {
			if (i == 0) {
				LEVEL.player_id = client_id;
			}
		} else {
			// At this point, the ServerUpdate is not yet processed
			if (i == GAME.client_thread().receiver_index()) {
				LEVEL.player_id = client_id;
			}
		}
	}

	// Add merchants
	auto active_merchants = pick_active_merchants(client_count);
	for (const auto& merchant_sprite : active_merchants) {
		// Lookup the location of the merchant
		auto posF = m2::VecF{_merchant_positions[merchant_sprite]} + m2::VecF{0.5f, 0.5f};
		// Create merchant object
		auto [merchant_obj, merchant_id] = m2::create_object(posF, m2g::pb::ObjectType::MERCHANT);
		cuzn::init_merchant(merchant_obj);
		// Store for later
		_merchant_object_ids[merchant_sprite] = merchant_id;
	}

	// Add market
	auto [market_obj, market_id] = m2::create_object(m2::VecF{}, m2g::pb::ObjectType::MARKET);
	cuzn::init_market(market_obj);
	_market_object_id = market_id;
}

void m2g::Proxy::multi_player_level_host_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	auto client_count = GAME.server_thread().client_count();

	// Prepare active merchant license list
	auto merchant_license_list = prepare_merchant_license_list(client_count);
	if (merchant_license_list.size() != _merchant_object_ids.size()) {
		throw M2ERROR("Merchant count and merchant license count mismatch");
	}
	// Assign licenses to merchants
	int i = 0;
	for (const auto& merchant_id : _merchant_object_ids) {
		auto license = merchant_license_list[i++];
		auto& merchant = LEVEL.objects[merchant_id.second];

		LOG_DEBUG("Adding license to merchant", m2g::pb::ItemType_Name(license));
		merchant.character().add_named_item(GAME.get_named_item(license));
		// Add beer to non-NO_LICENSE merchants
		if (license != m2g::pb::NO_MERCHANT_LICENSE) {
			merchant.character().add_resource(pb::BEER_BARREL_COUNT, 1.0f);
		}
	}

	// Initialize market
	_coal_market.emplace(cuzn::COAL_MARKET_INITIAL_COUNT, m2g::pb::COAL_CUBE_COUNT, _market_object_id);
	_iron_market.emplace(cuzn::IRON_MARKET_INITIAL_COUNT, m2g::pb::IRON_CUBE_COUNT, _market_object_id);

	// Prepare draw deck
	auto draw_deck = prepare_draw_deck(client_count);
	// In the canal era, we discard client_count number of cards from the deck
	m2_repeat(client_count) {
		draw_deck.pop_back();
	}

	// TODO
}

std::optional<int> m2g::Proxy::handle_client_command(unsigned turn_holder_index, MAYBE const m2g::pb::ClientCommand& client_command) {
	LOG_INFO("Received command from client", turn_holder_index);

	// Increment turn holder
	return (turn_holder_index + 1) % GAME.server_thread().client_count();
}

void m2g::Proxy::post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	// Store the positions of the merchants
    if (pb::GLOUCESTER_1 <= sprite_type && sprite_type <= pb::WARRINGTON_2) {
		auto pos = m2::VecI{floorf(obj.position.x), floorf(obj.position.y)};
		_merchant_positions[sprite_type] = pos;
		_position_merchants[pos] = sprite_type;
	}
}

m2::void_expected m2g::Proxy::init_fg_object(m2::Object& obj) {
	m2::void_expected init_result;
	switch (obj.object_type()) {
		default:
			return m2::make_unexpected("Invalid object type");
	}
	m2_reflect_failure(init_result);

	return {};
}

std::vector<m2g::pb::ItemType> m2g::Proxy::prepare_merchant_license_list(int client_count) {
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
		const auto& item = GAME.get_named_item(item_type);
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

std::vector<m2g::pb::SpriteType> m2g::Proxy::pick_active_merchants(int client_count) {
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

std::vector<m2g::pb::ItemType> m2g::Proxy::prepare_draw_deck(int client_count) {
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
		const auto& item = GAME.get_named_item(item_type);
		if (item.category() == pb::ITEM_CATEGORY_CARD) {
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
