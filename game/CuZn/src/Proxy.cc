#include <m2/Proxy.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Game.h>
#include <m2/multi_player/State.h>

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
}

void m2g::Proxy::multi_player_level_host_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	auto client_count = GAME.server_thread().client_count();

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

	// Prepare the draw deck
	std::vector<m2g::pb::ItemType> draw_deck;
	for (auto i = 0; i < m2::pb::enum_value_count<m2g::pb::ItemType>(); ++i) {
		auto item_type = m2::pb::enum_value<m2g::pb::ItemType>(i);
		const auto& item = GAME.get_named_item(item_type);
		if (item.category() == pb::ITEM_CATEGORY_CARD) {
			auto card_count = static_cast<int>(item.get_attribute(count_attr));
			draw_deck.insert(draw_deck.end(), card_count, item.type());
		}
	}
	LOG_DEBUG("DrawDeck", draw_deck);

	// TODO
}

std::optional<int> m2g::Proxy::handle_client_command(unsigned turn_holder_index, MAYBE const m2g::pb::ClientCommand& client_command) {
	LOG_INFO("Received command from client", turn_holder_index);

	// Increment turn holder
	return (turn_holder_index + 1) % GAME.server_thread().client_count();
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
