#include <m2g/Proxy.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/GameStateTracker.h>
#include <cuzn/object/Merchant.h>
#include <cuzn/object/Factory.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <cuzn/ui/Client.h>
#include <cuzn/ui/PauseMenu.h>
#include <cuzn/ui/MainMenu.h>
#include <cuzn/ui/LeftHud.h>
#include <cuzn/ui/RightHud.h>
#include <cuzn/detail/SetUp.h>
#include <m2/game/Detail.h>
#include "cuzn/object/Road.h"
#include "cuzn/ui/Detail.h"
#include <algorithm>
#include <numeric>

const m2::ui::Blueprint* m2g::Proxy::main_menu() { return &main_menu_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::pause_menu() { return &pause_menu_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::left_hud() { return &left_hud_blueprint; }

const m2::ui::Blueprint* m2g::Proxy::right_hud() { return &right_hud_blueprint; }

void m2g::Proxy::post_multi_player_level_client_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	DEBUG_FN();

	auto client_count =
	    M2_GAME.is_server() ? M2_GAME.server_thread().client_count() : M2_GAME.client_thread().total_player_count();

	// Add human players
	for (auto i = 0; i < client_count; ++i) {
		auto it = m2::create_object(m2::VecF{i, i}, m2g::pb::ObjectType::HUMAN_PLAYER);
		auto client_init_result = init_human_player(*it);
		m2_succeed_or_throw_error(client_init_result);
		multi_player_object_ids.emplace_back(it.id());
		player_colors.emplace_back(generate_player_color(i));

		if (i == M2_GAME.client_thread().receiver_index()) {
			M2_LEVEL.player_id = it.id();
		}
	}

	// Add all merchants (without license) since all merchants can deal in coal
	for (const auto& merchant_sprite : active_merchant_locations()) {
		// Create merchant object
		auto it = m2::create_object(merchant_positions[merchant_sprite].first, m2g::pb::ObjectType::MERCHANT);
		init_merchant(*it);
		// Store for later
		merchant_object_ids[merchant_sprite] = it.id();
	}

	// Add Game State Tracker object
	auto it = m2::create_object(m2::VecF{}, m2g::pb::ObjectType::GAME_STATE_TRACKER);
	init_game_state_tracker(*it);
	_game_state_tracker_id = it.id();
}

void m2g::Proxy::multi_player_level_server_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	auto client_count = M2_GAME.server_thread().client_count();

	// Assign licenses to active merchants
	{
		auto merchant_licenses = prepare_merchant_license_list(client_count);
		auto active_merchant_locs = active_merchant_locations(client_count);
		if (merchant_licenses.size() != active_merchant_locs.size()) {
			throw M2ERROR("Merchant count mismatch");
		}

		for (const auto& merchant_location : active_merchant_locs) {
			// Pop license from the list
			auto license = merchant_licenses.back();
			merchant_licenses.pop_back();
			const auto& license_item = M2_GAME.get_named_item(license);

			// Retrieve merchant object
			auto merchant_object_id = merchant_object_ids[merchant_location];
			auto& merchant_char = M2_LEVEL.objects[merchant_object_id].character();

			LOG_DEBUG("Adding license to merchant", m2g::pb::ItemType_Name(license));
			merchant_char.add_named_item(license_item);
			merchant_char.add_resource(pb::BEER_BARREL_COUNT, license_item.get_attribute(pb::BEER_BONUS_FIRST_ERA));
		}
	}

	// Prepare draw deck
	auto draw_deck = prepare_draw_deck(client_count);
	// In the canal era, we discard client_count number of cards from the deck
	m2_repeat(client_count) { draw_deck.pop_back(); }
	// Give 8 cards to each player
	for (const auto& player_object_id : M2G_PROXY.multi_player_object_ids) {
		m2_repeat(8) {
			// Draw card
			auto card = draw_deck.back();
			draw_deck.pop_back();
			// Add card
			M2_LEVEL.objects[player_object_id].character().add_named_item(M2_GAME.get_named_item(card));
		}
	}
	_draw_deck = std::move(draw_deck);
	// Store draw deck size to Game State Tracker
	game_state_tracker().set_resource(pb::DRAW_DECK_SIZE, m2::F(_draw_deck.size()));

	// Don't put the first player on the list
	for (int i = 1; i < M2_GAME.server_thread().client_count(); ++i) {
		_waiting_players.emplace_back(i);
	}
}

std::optional<int> m2g::Proxy::handle_client_command(int turn_holder_index, MAYBE const m2g::pb::ClientCommand& client_command) {
	LOG_INFO("Received command from client", turn_holder_index);
	auto turn_holder_object_id = M2G_PROXY.multi_player_object_ids[turn_holder_index];
	auto& turn_holder_character = M2_LEVEL.objects[turn_holder_object_id].character();

	std::optional<Card> card_to_discard;
	SpentMoney money_spent = 0;
	if (client_command.has_build_action()) {
		// TODO verify whether the player can build it

		card_to_discard = client_command.build_action().card();
		money_spent += 5;

		auto it = m2::create_object(
			position_of_industry_location(client_command.build_action().industry_location()),
			m2g::pb::FACTORY,
			turn_holder_object_id);
		auto success = init_factory(*it,
			city_of_location(client_command.build_action().industry_location()),
			client_command.build_action().industry_tile());
		// TODO check result
	} else if (client_command.has_network_action()) {
		// TODO verify whether the player can network

		card_to_discard = client_command.network_action().card();
		money_spent += 5;

		auto it = m2::create_object(
			position_of_connection(client_command.network_action().connection_1()),
			m2g::pb::ROAD,
			turn_holder_object_id);
		auto success = init_road(*it, client_command.network_action().connection_1());
		// TODO check result
	} else if (client_command.has_sell_action()) {
		// TODO check

		card_to_discard = client_command.sell_action().card();

		auto* factory = find_factory_at_location(client_command.sell_action().industry_location());
		// TODO check
		factory->character().set_resource(pb::IS_SOLD, 1.0f);
		// TODO get benefits
	} else if (client_command.has_develop_action()) {
		// TODO verify player can develop

		card_to_discard = client_command.develop_action().card();
		money_spent += 2;

		// Remove tiles from the player
		turn_holder_character.remove_item(turn_holder_character.find_items(client_command.develop_action().industry_tile_1()));
		if (client_command.develop_action().industry_tile_2()) {
			turn_holder_character.remove_item(turn_holder_character.find_items(client_command.develop_action().industry_tile_2()));
		}
		// Remove iron from industries or market
		if (is_industry_location(client_command.develop_action().iron_sources_1())) {
			find_factory_at_location(client_command.develop_action().iron_sources_1())->character().remove_resource(pb::IRON_CUBE_COUNT, 1.0f);
		} else if (is_merchant_location(client_command.develop_action().iron_sources_1())) {
			// TODO
		} else {
			throw M2ERROR("Invalid iron source");
		}
		// TODO remove duplication
		if (is_industry_location(client_command.develop_action().iron_sources_2())) {
			find_factory_at_location(client_command.develop_action().iron_sources_2())->character().remove_resource(pb::IRON_CUBE_COUNT, 1.0f);
		} else if (is_merchant_location(client_command.develop_action().iron_sources_2())) {
			// TODO
		} else {
			throw M2ERROR("Invalid iron source");
		}

	} else if (client_command.has_loan_action()) {
		// TODO check
		auto income_points = m2::iround(turn_holder_character.get_attribute(m2g::pb::INCOME_POINTS));
		auto income_level = level_from_income_points(income_points);
		auto new_income_level = std::max(-10, income_level - 3);
		auto new_income_points = highest_income_points_of_level(new_income_level);
		turn_holder_character.set_attribute(pb::INCOME_POINTS, static_cast<float>(new_income_points));
		turn_holder_character.add_resource(pb::MONEY, 30.0f);

		card_to_discard = client_command.loan_action().card();
	} else if (client_command.has_scout_action()) {
		// TODO check
		auto card_1_it = turn_holder_character.find_items(client_command.scout_action().card_1());
		auto card_2_it = turn_holder_character.find_items(client_command.scout_action().card_2());
		turn_holder_character.remove_item(card_1_it);
		turn_holder_character.remove_item(card_2_it);
		turn_holder_character.add_named_item(M2_GAME.get_named_item(pb::WILD_INDUSTRY_CARD));
		turn_holder_character.add_named_item(M2_GAME.get_named_item(pb::WILD_LOCATION_CARD));

		card_to_discard = client_command.scout_action().card_0();
	}

	// Send update to clients
	pb::ServerCommand sc;
	sc.set_display_blocking_message("Action taken");
	for (int i = 0; i < M2_GAME.server_thread().client_count(); ++i) {
		if (i != turn_holder_index) {
			M2_GAME.server_thread().send_server_command(sc, i);
		}
	}

	// Discard card from player
	if (card_to_discard) {
		auto card_it = turn_holder_character.find_items(*card_to_discard);
		turn_holder_character.remove_item(card_it);
	}
	// Record spent money
	if (not _played_players.empty() && _played_players.back().first == turn_holder_index) {
		_played_players.back().second += money_spent;
	} else {
		_played_players.emplace_back(turn_holder_index, money_spent);
	}

	// Count the cards in the game
	auto player_card_lists = M2G_PROXY.multi_player_object_ids
		| std::views::transform(m2::lookup_object_from_id)
		| std::views::transform(m2::to_character_of_object)
		| std::views::transform(m2::generate_named_item_types_transformer({pb::ITEM_CATEGORY_CITY_CARD, pb::ITEM_CATEGORY_INDUSTRY_CARD, pb::ITEM_CATEGORY_WILD_CARD}));
	auto card_count = std::accumulate(player_card_lists.begin(), player_card_lists.end(), (size_t)0, [](size_t sum, const std::vector<Card>& v) { return sum + v.size(); });
	card_count += _draw_deck.size();

	if (_waiting_players.empty() && card_count == 0) {
		// If no cards left in the game
		if (M2G_PROXY.is_canal_era()) {
			// TODO end era

			determine_player_orders();
		} else {
			// TODO end game
		}
	} else if (is_first_turn()) {
		// Draw single card
		auto card = _draw_deck.back();
		_draw_deck.pop_back();
		turn_holder_character.add_named_item(M2_GAME.get_named_item(card));

		// Check if first turn finished
		if (_waiting_players.empty()) {
			LOG_INFO("First turn ended");
			game_state_tracker().clear_resource(pb::IS_FIRST_TURN);

			determine_player_orders();
		}
	} else if (not is_first_turn() && card_count % 2) {
		// If there are odd number of cards, the turn holder does not change
		// Push to the front of the waiting players, so that it's popped first below.
		_waiting_players.push_front(turn_holder_index);
	} else {
		// If there are even number of cards, the turn holder changes.
		// Draw cards for the player
		if (not _draw_deck.empty()) {
			m2_repeat(2) {
				auto card = _draw_deck.back(); _draw_deck.pop_back();
				turn_holder_character.add_named_item(M2_GAME.get_named_item(card));
			}
		}

		if (_waiting_players.empty()) {
			determine_player_orders();
		}
	}

	// Store draw deck size to Game State Tracker
	game_state_tracker().set_resource(pb::DRAW_DECK_SIZE, m2::F(_draw_deck.size()));

	if (not _waiting_players.empty()) {
		auto tmp = _waiting_players.front(); _waiting_players.pop_front();
		return tmp;
	} else {
		// TODO Game ended
		return -1;
	}
}

void m2g::Proxy::handle_server_command(const pb::ServerCommand& server_command) {
	if (server_command.has_display_blocking_message()) {
		display_blocking_message(server_command.display_blocking_message(), "");
	} else {
		throw M2ERROR("Unsupported server command");
	}
}

void m2g::Proxy::post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	// Store the positions of the merchants
	if (is_merchant_location(sprite_type)) {
		// Object position has {0.5f, 0.5f} offset
		auto merchant_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 2.0f, 2.0f};
		merchant_positions[sprite_type] = std::make_pair(obj.position, merchant_cell_rect);
		LOG_DEBUG("Merchant position", m2g::pb::SpriteType_Name(sprite_type), merchant_cell_rect);
	}

	// Store the positions of the industries build locations
	else if (is_industry_location(sprite_type)) {
		// Verify that ppm of the industry tiles are double of the sprite sheet
		if (M2_GAME.get_sprite(sprite_type).ppm() != M2_GAME.get_sprite(sprite_type).sprite_sheet().sprite_sheet().ppm()) {
			throw M2ERROR("Sprite ppm mismatch");
		}
		// Object position has {0.5f, 0.5f} offset
		auto industry_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 2.0f, 2.0f};
		industry_positions[sprite_type] = std::make_pair(obj.position, industry_cell_rect);
		LOG_DEBUG("Industry position", m2g::pb::SpriteType_Name(sprite_type), industry_cell_rect);
	}

	// Store the positions of the connection locations
	else if (is_canal(sprite_type) || is_railroad(sprite_type)) {
		m2::RectF connection_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 1.0f, 1.0f};
		// Different canal or railroad backgrounds have different offsets
		auto original_type = M2_GAME.get_sprite(sprite_type).original_type();
		auto offset = connection_sprite_world_offset(*original_type);
		connection_cell_rect = connection_cell_rect.shift(offset);
		connection_positions[sprite_type] = std::make_pair(obj.position + offset, connection_cell_rect);
		LOG_DEBUG("Connection position", m2g::pb::SpriteType_Name(sprite_type), connection_cell_rect);
	}
}

m2::void_expected m2g::Proxy::init_level_blueprint_fg_object(MAYBE m2::Object& obj) {
	return m2::make_unexpected("Invalid object type");
}

m2::void_expected m2g::Proxy::init_server_update_fg_object(m2::Object& obj, const std::vector<m2g::pb::ItemType>& items,
	MAYBE const std::vector<m2::pb::Resource>& resources) {
	switch (obj.object_type()) {
		case pb::FACTORY: {
			auto city = std::ranges::find_if(items, is_city);
			auto industry_tile = std::ranges::find_if(items, is_industry_tile);
			if (city != items.end() && industry_tile != items.end()) {
				return init_factory(obj, *city, *industry_tile);
			} else {
				return m2::make_unexpected("Unable to find city or industry tile of the object received from the server");
			}
		}
		case pb::ROAD: {
			if (auto connection = connection_on_position(obj.position)) {
				return init_road(obj, *connection);
			} else {
				return m2::make_unexpected("Unable to find connection from object location");
			}
		}
		default:
			return m2::make_unexpected("Invalid object type");
	}
}

m2g::Proxy& m2g::Proxy::get_instance() {
	return dynamic_cast<m2g::Proxy&>(M2_GAME.proxy());
}

void m2g::Proxy::user_journey_deleter() {
	std::visit(m2::overloaded {
		[](auto& uj){ uj.deinit(); }
	}, *get_instance().user_journey);
	get_instance().user_journey.reset();
}

unsigned m2g::Proxy::player_index(m2::Id id) const {
	auto it = std::find(multi_player_object_ids.begin(), multi_player_object_ids.end(), id);
	if (it != multi_player_object_ids.end()) {
		return std::distance(multi_player_object_ids.begin(), it);
	} else {
		throw M2ERROR("Invalid player ID");
	}
}

m2::Character& m2g::Proxy::game_state_tracker() const {
	return M2_LEVEL.objects[_game_state_tracker_id].character();
}

bool m2g::Proxy::is_first_turn() const {
	return m2::is_equal(game_state_tracker().get_resource(pb::IS_FIRST_TURN), 1.0f, 0.001f);
}

bool m2g::Proxy::is_canal_era() const {
	return m2::is_equal(game_state_tracker().get_resource(pb::IS_RAILROAD_ERA), 0.0f, 0.001f);
}

bool m2g::Proxy::is_railroad_era() const {
	return m2::is_equal(game_state_tracker().get_resource(pb::IS_RAILROAD_ERA), 1.0f, 0.001f);
}

void m2g::Proxy::determine_player_orders() {
	if (not _waiting_players.empty()) {
		throw M2ERROR("Cannot determine player orders while there are players still waiting their turns");
	}

	while (not _played_players.empty()) {
		// Find the first player who spent the least amount of money
		SpentMoney spent_money = INT32_MAX;
		PlayerIndex player_index = _played_players.front().first; // If no one spent any money, move the first player
		for (auto [a_player_index, a_spent_money] : _played_players) {
			if (a_spent_money < spent_money) {
				spent_money = a_spent_money;
				player_index = a_player_index;
			}
		}

		// Remove the selected player from the list
		auto it = std::remove_if(_played_players.begin(), _played_players.end(), [player_index](const auto& pair) {
			return pair.first == player_index;
		});
		_played_players.erase(it);

		_waiting_players.emplace_back(player_index);
	}
}
