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
#include <cuzn/detail/Liquidate.h>
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
			merchant_char.add_resource(pb::BEER_BARREL_COUNT, 1.0f);
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

	// First turn holder is the server, enable action buttons
	enable_action_buttons();
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

		std::optional<Card> card_to_discard;
		SpentMoney money_spent = 0;
		if (client_command.has_build_action()) {
			LOG_INFO("Processing build action");
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
			LOG_INFO("Processing network action");
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
			LOG_INFO("Processing sell action");
			// TODO check

			card_to_discard = client_command.sell_action().card();

			auto* factory = find_factory_at_location(client_command.sell_action().industry_location());
			// TODO check
			factory->character().set_resource(pb::IS_SOLD, 1.0f);
			// TODO get benefits
		} else if (client_command.has_develop_action()) {
			LOG_INFO("Processing develop action");
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
			LOG_INFO("Processing loan action");
			// TODO check
			auto income_points = m2::iround(turn_holder_character.get_attribute(m2g::pb::INCOME_POINTS));
			auto income_level = income_level_from_income_points(income_points);
			auto new_income_level = std::max(-10, income_level - 3);
			auto new_income_points = highest_income_points_of_level(new_income_level);
			turn_holder_character.set_attribute(pb::INCOME_POINTS, static_cast<float>(new_income_points));
			turn_holder_character.add_resource(pb::MONEY, 30.0f);

			card_to_discard = client_command.loan_action().card();
		} else if (client_command.has_scout_action()) {
			LOG_INFO("Processing scout action");
			// TODO check
			auto card_1_it = turn_holder_character.find_items(client_command.scout_action().card_1());
			auto card_2_it = turn_holder_character.find_items(client_command.scout_action().card_2());
			turn_holder_character.remove_item(card_1_it);
			turn_holder_character.remove_item(card_2_it);
			turn_holder_character.add_named_item(M2_GAME.get_named_item(pb::WILD_INDUSTRY_CARD));
			turn_holder_character.add_named_item(M2_GAME.get_named_item(pb::WILD_LOCATION_CARD));

			card_to_discard = client_command.scout_action().card_0();
		} else if (client_command.has_pass_action()) {
			LOG_INFO("Processing pass action");
			card_to_discard = client_command.pass_action().card();
		}

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
			LOG_INFO("Discard card from player", M2_GAME.get_named_item(*card_to_discard).in_game_name());
			auto card_it = turn_holder_character.find_items(*card_to_discard);
			turn_holder_character.remove_item(card_it);
		}

		// Record spent money
		if (not _played_players.empty() && _played_players.back().first == turn_holder_index) {
			_played_players.back().second += money_spent;
		} else {
			_played_players.emplace_back(turn_holder_index, money_spent);
		}
	}

	// Determine next player

	// Count the cards in the game
	auto player_card_lists = M2G_PROXY.multi_player_object_ids
		| std::views::transform(m2::to_object_with_id)
		| std::views::transform(m2::to_character_of_object)
		| std::views::transform(m2::generate_named_item_types_transformer({pb::ITEM_CATEGORY_CITY_CARD, pb::ITEM_CATEGORY_INDUSTRY_CARD, pb::ITEM_CATEGORY_WILD_CARD}));
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
					| std::views::transform(m2::to_object_with_id)
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
						auto& player = factory.parent();
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

	if (next_turn_holder == 0) {
		enable_action_buttons();
	} else {
		disable_action_buttons();
	}
	return next_turn_holder;
}

void m2g::Proxy::handle_server_command(const pb::ServerCommand& server_command) {
	if (server_command.has_display_blocking_message()) {
		display_blocking_message(server_command.display_blocking_message(), "");
	} else if (server_command.has_liquidate_assets_for_loan()) {
		LOG_INFO("Received liquidate command, beginning liquidation journey");
		auto money_to_be_paid = server_command.liquidate_assets_for_loan();
		M2G_PROXY.user_journey.emplace(LiquidationJourney{money_to_be_paid});
	} else {
		throw M2ERROR("Unsupported server command");
	}
}

void m2g::Proxy::post_server_update(MAYBE const m2::pb::ServerUpdate& server_update) {
	if (M2_GAME.client_thread().is_turn()) {
		enable_action_buttons();
	} else {
		disable_action_buttons();
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

namespace {
	std::initializer_list<std::string> action_button_names = {"BuildButton", "NetworkButton", "DevelopButton",
		"SellButton", "LoanButton", "ScoutButton", "PassButton"};
}

void m2g::Proxy::enable_action_buttons() {
	for (const auto& button_name : action_button_names) {
		auto* button = M2_LEVEL.left_hud_ui_state->find_first_widget_by_name<m2::ui::widget::Text>(button_name);
		button->enabled = true;
	}
}

void m2g::Proxy::disable_action_buttons() {
	for (const auto& button_name : action_button_names) {
		auto* button = M2_LEVEL.left_hud_ui_state->find_first_widget_by_name<m2::ui::widget::Text>(button_name);
		button->enabled = false;
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

	// Liquidation not necessary
	return std::nullopt;
}
