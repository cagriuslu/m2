#include <m2g/Proxy.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/GameStateTracker.h>
#include <cuzn/detail/Income.h>
#include <cuzn/object/Merchant.h>
#include <cuzn/object/Factory.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextInput.h>
#include <cuzn/ui/Client.h>
#include <cuzn/ui/PauseMenu.h>
#include <cuzn/ui/MainMenu.h>
#include <cuzn/ui/LeftHud.h>
#include <cuzn/ui/RightHud.h>
#include <cuzn/detail/SetUp.h>
#include <cuzn/detail/Market.h>
#include <cuzn/journeys/ScoutJourney.h>
#include <cuzn/journeys/LoanJourney.h>
#include <cuzn/detail/Liquidate.h>
#include <m2/game/Detail.h>
#include "cuzn/object/Road.h"
#include "cuzn/ui/Detail.h"
#include <algorithm>
#include <numeric>
#include <cuzn/ui/CustomHud.h>
#include <cuzn/ui/StatusBar.h>
#include <cuzn/ui/Notification.h>

const m2::ui::PanelBlueprint* m2g::Proxy::main_menu() { return &main_menu_blueprint; }

const m2::ui::PanelBlueprint* m2g::Proxy::pause_menu() { return &pause_menu_blueprint; }

const m2::ui::PanelBlueprint* m2g::Proxy::left_hud() { return &left_hud_blueprint; }

const m2::ui::PanelBlueprint* m2g::Proxy::right_hud() { return &right_hud_blueprint; }

void m2g::Proxy::post_multi_player_level_client_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	DEBUG_FN();

	auto client_count = M2_GAME.total_player_count();
	auto self_index = M2_GAME.self_index();

	// Add human players
	for (auto i = 0; i < client_count; ++i) {
		auto it = m2::create_object(m2::VecF{i, i}, m2g::pb::ObjectType::HUMAN_PLAYER);
		auto client_init_result = init_human_player(*it);
		m2_succeed_or_throw_error(client_init_result);
		multi_player_object_ids.emplace_back(it.id());
		player_colors.emplace_back(generate_player_color(i));

		if (i == self_index) {
			M2_LEVEL.player_id = it.id();
		}
	}

	// Add all merchants (without any license) since all merchants can deal in coal and iron
	for (const auto& merchant_sprite : possibly_active_merchant_locations()) {
		// Create merchant object
		auto it = m2::create_object(std::get<m2::VecF>(merchant_positions[merchant_sprite]), m2g::pb::ObjectType::MERCHANT);
		init_merchant(*it);
		// Store for later
		merchant_object_ids[merchant_sprite] = it.id();
	}

	// Add Game State Tracker object
	auto it = m2::create_object(m2::VecF{}, m2g::pb::ObjectType::GAME_STATE_TRACKER);
	init_game_state_tracker(*it);
	_game_state_tracker_id = it.id();

	// Add status bar panel to the level
	_status_bar_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(
		m2::ui::Panel{
			std::make_unique<m2::ui::PanelBlueprint>(generate_status_bar_blueprint(client_count)),
			status_bar_window_ratio()
		});
}

void m2g::Proxy::multi_player_level_server_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	auto client_count = M2_GAME.server_thread().client_count();

	// Assign licenses to active merchants
	{
		auto merchant_licenses = prepare_merchant_license_list(client_count); // Contains some active licenses, and some NO_MERCHANT_LICENSE
		auto possibly_active_merchant_locs = possibly_active_merchant_locations(client_count);
		if (merchant_licenses.size() != possibly_active_merchant_locs.size()) {
			throw M2_ERROR("Merchant count mismatch");
		}

		for (const auto& possibly_active_merchant_loc : possibly_active_merchant_locs) {
			// Pop license from the list
			auto license = merchant_licenses.back();
			merchant_licenses.pop_back();
			if (license != pb::NO_MERCHANT_LICENSE) {
				// Retrieve merchant object
				auto merchant_object_id = merchant_object_ids[possibly_active_merchant_loc];
				auto& merchant_char = M2_LEVEL.objects[merchant_object_id].character();
				LOG_DEBUG("Adding license to merchant", m2g::pb::ItemType_Name(license));
				merchant_char.add_named_item(M2_GAME.get_named_item(license));
				merchant_char.add_resource(pb::BEER_BARREL_COUNT, 1.0f);
			}
		}
	}

	// Prepare draw deck
	auto draw_deck = prepare_draw_deck(client_count);
	m2_repeat(client_count) { draw_deck.pop_back(); } // In the canal era, we discard client_count number of cards from the deck
	give_8_cards_to_each_player(draw_deck);
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

	if (is_liquidating()) {
		if (not client_command.has_liquidate_action()) {
			LOG_WARN("Received unexpected command while expecting LiquidateAction");
			return std::nullopt;
		} else if (auto expect_factories_and_gain = can_player_liquidate_factories(turn_holder_character, client_command.liquidate_action())) {
			LOG_INFO("Liquidating factories");
			for (auto* factory : expect_factories_and_gain->first) {
				// Delete object immediately
				M2_LEVEL.objects.free(factory->id());
			}
			// Gain money
			turn_holder_character.add_resource(pb::MONEY, m2::F(expect_factories_and_gain->second));
			// No longer liquidating
			set_is_liquidating(false);
		} else {
			LOG_WARN("Player sent invalid liquidate command", expect_factories_and_gain.error());
			return std::nullopt;
		}
	} else {
		// LiquidateAction is not allowed
		if (client_command.has_liquidate_action()) {
			LOG_WARN("Received unexpected LiquidateAction");
			return std::nullopt;
		}

		std::pair<Card,int> card_to_discard_and_money_spent{};
		if (client_command.has_build_action()) {
			LOG_INFO("Validating build action");
			if (not can_player_build(turn_holder_character, client_command.build_action())) {
				return std::nullopt;
			}
			LOG_INFO("Executing build action");
			card_to_discard_and_money_spent = execute_build_action(turn_holder_character, client_command.build_action());
		} else if (client_command.has_network_action()) {
			LOG_INFO("Validating network action");
			if (not can_player_network(turn_holder_character, client_command.network_action())) {
				return std::nullopt;
			}
			LOG_INFO("Executing network action");
			card_to_discard_and_money_spent = execute_network_action(turn_holder_character, client_command.network_action());
		} else if (client_command.has_sell_action()) {
			LOG_INFO("Validating sell action");
			if (auto success = can_player_sell(turn_holder_character, client_command.sell_action()); not success) {
				LOG_WARN("Sell validation failed", success.error());
				return std::nullopt;
			}
			LOG_INFO("Executing sell action");
			card_to_discard_and_money_spent.first = execute_sell_action(turn_holder_character, client_command.sell_action());
		} else if (client_command.has_develop_action()) {
			LOG_INFO("Validating develop action");
			if (not can_player_develop(turn_holder_character, client_command.develop_action())) {
				return std::nullopt;
			}
			LOG_INFO("Executing develop action");
			card_to_discard_and_money_spent = execute_develop_action(turn_holder_character, client_command.develop_action());
		} else if (client_command.has_loan_action()) {
			LOG_INFO("Validating loan action");
			if (not can_player_loan(turn_holder_character, client_command.loan_action())) {
				return std::nullopt;
			}
			LOG_INFO("Executing loan action");
			card_to_discard_and_money_spent.first = execute_loan_action(turn_holder_character, client_command.loan_action());
		} else if (client_command.has_scout_action()) {
			LOG_INFO("Validating scout action");
			if (not can_player_scout(turn_holder_character, client_command.scout_action())) {
				return std::nullopt;
			}
			LOG_INFO("Executing scout action");
			card_to_discard_and_money_spent.first = execute_scout_action(turn_holder_character, client_command.scout_action());
		} else if (client_command.has_pass_action()) {
			LOG_INFO("Executing pass action");
			card_to_discard_and_money_spent.first = client_command.pass_action().card();
		}
		auto [card_to_discard, money_spent] = card_to_discard_and_money_spent;

		// Send update to clients
		LOG_DEBUG("Sending action summary to clients");
		pb::ServerCommand sc;
		sc.set_display_blocking_message("Action taken");
		for (int i = 0; i < M2_GAME.server_thread().client_count(); ++i) {
			if (i != turn_holder_index) {
				M2_GAME.server_thread().send_server_command(sc, i);
			}
		}

		// Discard card from player
		if (card_to_discard) {
			LOG_INFO("Discard card from player", M2_GAME.get_named_item(card_to_discard).in_game_name());
			auto card_it = turn_holder_character.find_items(card_to_discard);
			turn_holder_character.remove_item(card_it);
		}

		// Record spent money
		if (not _played_players.empty() && _played_players.back().first == turn_holder_index) {
			_played_players.back().second += money_spent;
		} else {
			_played_players.emplace_back(turn_holder_index, money_spent);
		}
		// Deduct money from player
		turn_holder_character.remove_resource(pb::MONEY, m2::F(money_spent));

		// Save spent money to game state tracker so that it's shared among clients
		for (int i = 0; i < M2_GAME.total_player_count(); ++i) {
			auto money_spent_by_player_enum = static_cast<pb::ResourceType>(pb::MONEY_SPENT_BY_PLAYER_0 + i);
			game_state_tracker().set_resource(money_spent_by_player_enum, 0.0f);
		}
		for (auto [player_index, spent_money] : _played_players) {
			auto money_spent_by_player_enum = static_cast<pb::ResourceType>(pb::MONEY_SPENT_BY_PLAYER_0 + player_index);
			game_state_tracker().set_resource(money_spent_by_player_enum, m2::F(spent_money));
		}
	}

	// Determine next player

	// Count the cards in the game
	auto player_card_lists = M2G_PROXY.multi_player_object_ids
		| std::views::transform(m2::to_object_of_id)
		| std::views::transform(m2::to_character_of_object)
		| std::views::transform(m2::generate_named_item_types_filter({pb::ITEM_CATEGORY_CITY_CARD, pb::ITEM_CATEGORY_INDUSTRY_CARD, pb::ITEM_CATEGORY_WILD_CARD}));
	auto card_count = std::accumulate(player_card_lists.begin(), player_card_lists.end(), (size_t)0, [](size_t sum, const std::vector<Card>& v) { return sum + v.size(); });
	card_count += _draw_deck.size();

	std::optional<std::pair<PlayerIndex, m2g::pb::ServerCommand>> liquidation_necessary;
	if (_waiting_players.empty() && card_count == 0) {
		LOG_INFO("No cards left in the game");
		// If no cards left in the game
		if (M2G_PROXY.is_canal_era()) {
			liquidation_necessary = prepare_next_round();
			if (not liquidation_necessary) {
				LOG_INFO("Ending canal era");

				// Score links
				std::ranges::for_each(
					M2G_PROXY.multi_player_object_ids
					| std::views::transform(m2::to_object_of_id)
					| std::views::transform(m2::to_character_of_object),
					[](m2::Character& player) {
						player.add_resource(pb::VICTORY_POINTS, m2::F(player_link_count(player)));
					});
				remove_all_roads();

				// Score sold factories
				std::ranges::for_each(M2_LEVEL.characters
					| std::views::transform(m2::to_character_base)
					| std::views::filter(is_factory_character)
					| std::views::filter(is_factory_sold),
					[](m2::Character& factory) {
						auto& player = factory.owner();
						player.character().add_resource(pb::VICTORY_POINTS, factory.get_attribute(pb::VICTORY_POINTS_BONUS));
					});
				remove_obsolete_factories();

				// Reset merchant beer
				for (const auto& [_, merchant_id] : merchant_object_ids) {
					M2_LEVEL.objects[merchant_id].character().add_resource(pb::BEER_BARREL_COUNT, 1.0f);
				}

				// Shuffle the draw deck
				auto draw_deck = prepare_draw_deck(M2_GAME.server_thread().client_count());
				give_8_cards_to_each_player(draw_deck);
				_draw_deck = std::move(draw_deck);
				// Store draw deck size to Game State Tracker
				game_state_tracker().set_resource(pb::DRAW_DECK_SIZE, m2::F(_draw_deck.size()));
				
				game_state_tracker().set_resource(pb::IS_RAILROAD_ERA, 1.0f);

				liquidation_necessary = prepare_next_round();
			}
		} else {
			LOG_INFO("Ending game");
			// TODO end game
		}
	} else if (is_first_turn()) {
		// Give card to player
		auto card = _draw_deck.back();
		_draw_deck.pop_back();
		turn_holder_character.add_named_item(M2_GAME.get_named_item(card));

		// Check if first turn finished
		if (_waiting_players.empty()) {
			LOG_INFO("First turn ended");
			game_state_tracker().clear_resource(pb::IS_FIRST_TURN);

			liquidation_necessary = prepare_next_round();
		} else {
			// Otherwise, just fetch the next player from _waiting_players
			LOG_INFO("Switch to next player");
		}
	} else if (not is_first_turn() && card_count % 2) {
		// If there are odd number of cards, the turn holder does not change
		// Push to the front of the waiting players, so that it's popped first below.
		_waiting_players.push_front(turn_holder_index);
		LOG_INFO("Continuing with the same player");
	} else {
		// If there are even number of cards, the turn holder changes.
		// Give cards to player
		while (not _draw_deck.empty() && player_card_count(turn_holder_character) < 8) {
			auto card = _draw_deck.back();
			_draw_deck.pop_back();
			turn_holder_character.add_named_item(M2_GAME.get_named_item(card));
		}

		// Try to prepare the next round.
		if (_waiting_players.empty()) {
			liquidation_necessary = prepare_next_round();
		} else {
			// Otherwise, just fetch the next player from _waiting_players
			LOG_INFO("Switch to next player");
		}
	}

	// Store draw deck size to Game State Tracker
	game_state_tracker().set_resource(pb::DRAW_DECK_SIZE, m2::F(_draw_deck.size()));

	int next_turn_holder;
	if (liquidation_necessary) {
		set_is_liquidating(true); // Set the liquidation state so that client commands are handled properly
		LOG_INFO("Sending liquidation command to player", liquidation_necessary->first);
		M2_GAME.server_thread().send_server_command(liquidation_necessary->second, liquidation_necessary->first);
		// Give turn holder index to that player so that they can respond
		next_turn_holder = liquidation_necessary->first;
	} else {
		if (not _waiting_players.empty()) {
			next_turn_holder = _waiting_players.front();
			_waiting_players.pop_front();
		} else {
			// TODO Game ended
			next_turn_holder = -1;
		}
	}

	return next_turn_holder;
}

void m2g::Proxy::handle_server_command(const pb::ServerCommand& server_command) {
	if (server_command.has_display_blocking_message()) {
		display_blocking_message(server_command.display_blocking_message());
	} else if (server_command.has_liquidate_assets_for_loan()) {
		LOG_INFO("Received liquidate command, beginning liquidation journey");
		auto money_to_be_paid = server_command.liquidate_assets_for_loan();
		M2G_PROXY.user_journey.emplace(LiquidationJourney{money_to_be_paid});
	} else {
		throw M2_ERROR("Unsupported server command");
	}
}

void m2g::Proxy::post_server_update(bool shutdown) {
	// Delete the custom hud and refresh the status bar
	if (custom_hud_panel) {
		M2_LEVEL.remove_custom_nonblocking_ui_panel(*custom_hud_panel);
		custom_hud_panel = std::nullopt;
	}
	*_status_bar_panel = m2::ui::Panel{
		std::make_unique<m2::ui::PanelBlueprint>(generate_status_bar_blueprint(M2_GAME.total_player_count())),
		status_bar_window_ratio()
	};

	// Enable/disable buttons
	if (M2_GAME.is_our_turn()) {
		enable_action_buttons();
	} else {
		disable_action_buttons();
	}

	// TODO handle shutdown
}

void m2g::Proxy::bot_handle_server_update(const m2::pb::ServerUpdate& server_update) {
	auto receiver_index = server_update.receiver_index();
	// If it's bot's turn, pass turn
	if (receiver_index == server_update.turn_holder_index()) {
		auto player_object_id = server_update.player_object_ids(receiver_index);
		auto object_it = std::find_if(server_update.objects_with_character().begin(), server_update.objects_with_character().end(), [player_object_id](const m2::pb::ServerUpdate_ObjectDescriptor& obj_desc) {
			return obj_desc.object_id() == player_object_id;
		});
		// Find any card
		auto card_it = std::find_if(object_it->named_items().begin(), object_it->named_items().end(), [](int item_type) {
			return is_card(static_cast<m2g::pb::ItemType>(item_type));
		});
		// Pass turn
		LOG_INFO("Bot passing turn");
		m2g::pb::ClientCommand cc;
		cc.mutable_pass_action()->set_card(static_cast<m2g::pb::ItemType>(*card_it));
		M2_GAME.find_bot(receiver_index).queue_client_command(cc);
	}
}

void m2g::Proxy::bot_handle_server_command(MAYBE const m2g::pb::ServerCommand& server_command, MAYBE int receiver_index) {
	INFO_FN();
}

void m2g::Proxy::post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	// Store the positions of the merchants
	if (is_merchant_location(sprite_type)) {
		// Object position has {0.5f, 0.5f} offset
		auto merchant_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 2.0f, 2.0f};
		merchant_positions[sprite_type] = std::make_tuple(obj.position, merchant_cell_rect, obj.id());
		LOG_DEBUG("Merchant position", m2g::pb::SpriteType_Name(sprite_type), merchant_cell_rect);
	}
	// Store the positions of the industries build locations
	else if (is_industry_location(sprite_type)) {
		// Verify that ppm of the industry tiles are double of the sprite sheet
		if (M2_GAME.get_sprite(sprite_type).ppm() != M2_GAME.get_sprite(sprite_type).sprite_sheet().sprite_sheet().ppm()) {
			throw M2_ERROR("Sprite ppm mismatch");
		}
		// Object position has {0.5f, 0.5f} offset
		auto industry_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 2.0f, 2.0f};
		industry_positions[sprite_type] = std::make_tuple(obj.position, industry_cell_rect, obj.id());
		LOG_DEBUG("Industry position", m2g::pb::SpriteType_Name(sprite_type), industry_cell_rect);
	}
	// Store the positions of the connection locations
	else if (is_canal(sprite_type) || is_railroad(sprite_type)) {
		m2::RectF connection_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 1.0f, 1.0f};
		// Different canal or railroad backgrounds have different offsets
		auto original_type = M2_GAME.get_sprite(sprite_type).original_type(); // Connection sprites are duplicate of another
		auto offset = connection_sprite_world_offset(*original_type);
		connection_cell_rect = connection_cell_rect.shift(offset);
		connection_positions[sprite_type] = std::make_tuple(obj.position + offset, connection_cell_rect, obj.id());
		LOG_DEBUG("Connection position", m2g::pb::SpriteType_Name(sprite_type), connection_cell_rect);
		// Fill graph
		auto cities = cities_from_connection(sprite_type);
		if (cities.size() == 2) {
			available_connections_graph.add_edge(cities[0], {cities[1], 1.0f});
			available_connections_graph.add_edge(cities[1], {cities[0], 1.0f});
		} else if (cities.size() == 3) {
			available_connections_graph.add_edge(cities[0], {cities[1], 1.0f});
			available_connections_graph.add_edge(cities[1], {cities[0], 1.0f});
			available_connections_graph.add_edge(cities[0], {cities[2], 1.0f});
			available_connections_graph.add_edge(cities[2], {cities[0], 1.0f});
			available_connections_graph.add_edge(cities[1], {cities[2], 1.0f});
			available_connections_graph.add_edge(cities[2], {cities[1], 1.0f});
		} else {
			throw M2_ERROR("Invalid connection");
		}
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

void m2g::Proxy::main_journey_deleter() {
	std::visit(m2::overloaded {
		[](auto& uj){ uj.deinit(); }
	}, *get_instance().main_journeys);
	get_instance().main_journeys.reset();
}

unsigned m2g::Proxy::player_index(m2::Id id) const {
	auto it = std::find(multi_player_object_ids.begin(), multi_player_object_ids.end(), id);
	if (it != multi_player_object_ids.end()) {
		return std::distance(multi_player_object_ids.begin(), it);
	} else {
		throw M2_ERROR("Invalid player ID");
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

bool m2g::Proxy::is_liquidating() const {
	return m2::is_equal(game_state_tracker().get_resource(m2g::pb::IS_LIQUIDATING), 1.0f, 0.001f);
}

int m2g::Proxy::market_coal_count() const {
	return m2::iround(game_state_tracker().get_resource(m2g::pb::COAL_CUBE_COUNT));
}

int m2g::Proxy::market_iron_count() const {
	return m2::iround(game_state_tracker().get_resource(m2g::pb::IRON_CUBE_COUNT));
}

int m2g::Proxy::market_coal_cost(int coal_count) const {
	auto current_coal_count = m2::iround(game_state_tracker().get_resource(m2g::pb::COAL_CUBE_COUNT));
	return calculate_cost(COAL_MARKET_CAPACITY, current_coal_count, coal_count);
}

int m2g::Proxy::market_iron_cost(int iron_count) const {
	auto current_iron_count = m2::iround(game_state_tracker().get_resource(m2g::pb::IRON_CUBE_COUNT));
	return calculate_cost(IRON_MARKET_CAPACITY, current_iron_count, iron_count);
}

int m2g::Proxy::player_spent_money(int player_index) const {
	auto money_spent_by_player_enum = static_cast<pb::ResourceType>(pb::MONEY_SPENT_BY_PLAYER_0 + player_index);
	return m2::iround(game_state_tracker().get_resource(money_spent_by_player_enum));
}

std::pair<int,int> m2g::Proxy::market_coal_revenue(int count) const {
	auto current_coal_count = m2::iround(game_state_tracker().get_resource(m2g::pb::COAL_CUBE_COUNT));
	return calculate_revenue(COAL_MARKET_CAPACITY, current_coal_count, count);
}

std::pair<int,int> m2g::Proxy::market_iron_revenue(int count) const {
	auto current_iron_count = m2::iround(game_state_tracker().get_resource(m2g::pb::IRON_CUBE_COUNT));
	return calculate_revenue(IRON_MARKET_CAPACITY, current_iron_count, count);
}

std::set<m2::ObjectId> m2g::Proxy::object_ids_of_industry_location_bg_tiles(const std::set<IndustryLocation>& industry_locations) const {
	std::set<m2::ObjectId> ids;
	for (const auto& industry_location : industry_locations) {
		ids.insert(std::get<m2::ObjectId>(M2G_PROXY.industry_positions[industry_location]));
	}
	return ids;
}

std::set<m2::ObjectId> m2g::Proxy::object_ids_of_connection_bg_tiles(const std::set<Connection>& connections) const {
	std::set<m2::ObjectId> ids;
	for (const auto& connection : connections) {
		ids.insert(std::get<m2::ObjectId>(M2G_PROXY.connection_positions[connection]));
	}
	return ids;
}

namespace {
	std::initializer_list<std::string> action_button_names = {"BuildButton", "NetworkButton", "DevelopButton",
		"SellButton", "LoanButton", "ScoutButton", "PassButton"};
}

void m2g::Proxy::enable_action_buttons() {
	for (const auto& button_name : action_button_names) {
		auto* button = M2_LEVEL.left_hud_ui_panel->find_first_widget_by_name<m2::ui::widget::Text>(button_name);
		button->enabled = true;
	}
}

void m2g::Proxy::disable_action_buttons() {
	for (const auto& button_name : action_button_names) {
		auto* button = M2_LEVEL.left_hud_ui_panel->find_first_widget_by_name<m2::ui::widget::Text>(button_name);
		button->enabled = false;
	}
}

void m2g::Proxy::show_notification(const std::string& msg) {
	remove_notification();
	_notification_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(
		m2::ui::Panel{std::make_unique<m2::ui::PanelBlueprint>(generate_notification_panel_blueprint(msg)),
		    m2::RectF{0.1f, 0.96f, 0.8f, 0.04f}});
}
void m2g::Proxy::remove_notification() {
	if (_notification_panel) {
		M2_LEVEL.remove_custom_nonblocking_ui_panel(*_notification_panel);
		_notification_panel.reset();
	}
}

std::optional<std::pair<m2g::Proxy::PlayerIndex, m2g::pb::ServerCommand>> m2g::Proxy::prepare_next_round() {
	LOG_INFO("Prepare next round");

	// First, before preparing the next round, check if liquidation is necessary.
	if (auto liquidation = is_liquidation_necessary()) {
		LOG_INFO("Liquidation is necessary");
		// Prepare the ServerCommand and return
		m2g::pb::ServerCommand sc;
		sc.set_liquidate_assets_for_loan(liquidation->second);
		return std::make_pair(liquidation->first, sc);
	}

	// Gain incomes
	for (auto player_id : M2G_PROXY.multi_player_object_ids) {
		// Lookup player
		auto& player_character = M2_LEVEL.objects[player_id].character();
		auto income_points = m2::iround(player_character.get_attribute(pb::INCOME_POINTS));
		auto income_level = income_level_from_income_points(income_points);
		auto player_money = m2::iround(player_character.get_resource(pb::MONEY));
		LOG_DEBUG("Player gained money", income_level);
		auto new_player_money = player_money + income_level;
		if (new_player_money < 0) {
			LOG_INFO("Player doesn't have enough money to pay its loan, they'll lose victory points", new_player_money);
			player_character.add_resource(pb::VICTORY_POINTS, m2::F(new_player_money));
		}
		player_character.set_resource(pb::MONEY, m2::F(std::clamp(new_player_money, 0, INT32_MAX)));
	}

	// Determine player orders
	if (not _waiting_players.empty()) {
		throw M2_ERROR("Cannot determine player orders while there are players still waiting their turns");
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

	// Liquidation not necessary
	return std::nullopt;
}

void m2g::Proxy::set_is_liquidating(bool state) {
	game_state_tracker().set_resource(m2g::pb::IS_LIQUIDATING, state ? 1.0f : 0.0f);
}

void m2g::Proxy::buy_coal_from_market() {
	game_state_tracker().remove_resource(pb::COAL_CUBE_COUNT, 1.0f);
}

void m2g::Proxy::buy_iron_from_market() {
	game_state_tracker().remove_resource(pb::IRON_CUBE_COUNT, 1.0f);
}

void m2g::Proxy::sell_coal_to_market(int count) {
	game_state_tracker().add_resource(pb::COAL_CUBE_COUNT, m2::F(count));
}

void m2g::Proxy::sell_iron_to_market(int count) {
	game_state_tracker().add_resource(pb::IRON_CUBE_COUNT, m2::F(count));
}
