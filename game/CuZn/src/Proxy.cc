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
#include <cuzn/ui/GameResult.h>
#include <cuzn/ui/PauseMenu.h>
#include <cuzn/ui/MainMenu.h>
#include <cuzn/ui/LeftHud.h>
#include <cuzn/ui/RightHud.h>
#include <cuzn/detail/SetUp.h>
#include <cuzn/detail/Market.h>
#include <cuzn/detail/Liquidate.h>
#include "cuzn/object/Road.h"
#include "cuzn/ui/Detail.h"
#include <algorithm>
#include <cuzn/Scoring.h>
#include <cuzn/ui/ActionNotification.h>
#include <numeric>
#include <cuzn/ui/CustomHud.h>
#include <cuzn/ui/StatusBar.h>
#include <cuzn/ui/CanalEraResult.h>
#include <cuzn/ui/Cards.h>
#include <cuzn/Action.h>

using namespace m2;
using namespace m2g;

const m2::UiPanelBlueprint* m2g::Proxy::MainMenuBlueprint() { return &main_menu_blueprint; }

const m2::UiPanelBlueprint* m2g::Proxy::PauseMenuBlueprint() { return &pause_menu_blueprint; }

const m2::UiPanelBlueprint* m2g::Proxy::LeftHudBlueprint() { return &left_hud_blueprint; }

const m2::UiPanelBlueprint* m2g::Proxy::RightHudBlueprint() { return &right_hud_blueprint; }

void m2g::Proxy::post_multi_player_level_client_init(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	DEBUG_FN();

	auto client_count = M2_GAME.TotalPlayerCount();
	auto self_index = M2_GAME.SelfIndex();

	// Add human players
	for (auto i = 0; i < client_count; ++i) {
		auto it = m2::create_object(m2::VecF{i, i}, m2g::pb::ObjectType::HUMAN_PLAYER);
		if (i == self_index) {
			auto client_init_result = PlayerInitThisInstance(*it);
			m2_succeed_or_throw_error(client_init_result);
		} else {
			auto client_init_result = PlayerInitOtherInstance(*it);
			m2_succeed_or_throw_error(client_init_result);
		}
		multiPlayerObjectIds.emplace_back(it.GetId());
		player_colors.emplace_back(generate_player_color(i));
	}

	// Add all merchants (without any license) since all merchants can deal in coal and iron
	for (const auto& merchant_sprite : PossiblyActiveMerchantLocations()) {
		// Create merchant object
		auto it = m2::create_object(std::get<m2::VecF>(merchant_positions[merchant_sprite]), m2g::pb::ObjectType::MERCHANT);
		init_merchant(*it);
		// Store for later
		merchant_object_ids[merchant_sprite] = it.GetId();
	}

	// Add Game State Tracker object
	auto it = m2::create_object(m2::VecF{}, m2g::pb::ObjectType::GAME_STATE_TRACKER);
	InitGameStateTracker(*it);
	_game_state_tracker_id = it.GetId();

	// Add status bar panel to the level
	_status_bar_panel = M2_LEVEL.AddCustomNonblockingUiPanel(
		std::make_unique<m2::UiPanelBlueprint>(generate_status_bar_blueprint()),
		status_bar_window_ratio());
}

void m2g::Proxy::multi_player_level_server_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level) {
	auto client_count = M2_GAME.ServerThread().client_count();

	// Assign licenses to active merchants
	{
		auto merchant_licenses = PrepareMerchantLicenseList(client_count); // Contains some active licenses, and some NO_MERCHANT_LICENSE
		auto possibly_active_merchant_locs = PossiblyActiveMerchantLocations(client_count);
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
				merchant_char.AddNamedItem(M2_GAME.GetNamedItem(license));
				merchant_char.AddResource(pb::BEER_BARREL_COUNT, 1.0f);
			}
		}
	}

	// Prepare draw deck
	auto draw_deck = PrepareDrawDeck(client_count);
	m2_repeat(client_count) { draw_deck.pop_back(); } // In the canal era, we discard client_count number of cards from the deck
	Give8CardsToEachPlayer(draw_deck);
	_draw_deck = std::move(draw_deck);
	game_state_tracker().SetResource(pb::DRAW_DECK_SIZE, m2::F(_draw_deck.size()));

	// Don't put the first player on the list
	for (int i = 1; i < M2_GAME.ServerThread().client_count(); ++i) {
		_waiting_players.emplace_back(i);
	}
	// Now that the player order is determined, fill the game state tracker with the order
	{
		game_state_tracker().SetAttribute(pb::FIRST_PLAYER_INDEX, 0.0f);
		auto waitingPlayersCopy = _waiting_players;
		for (auto order = pb::SECOND_PLAYER_INDEX;
				order <= pb::FORTH_PLAYER_INDEX;
				order = static_cast<pb::AttributeType>(I(order) + 1)) {
			if (not waitingPlayersCopy.empty()) {
				game_state_tracker().SetAttribute(order, F(waitingPlayersCopy.front()));
				waitingPlayersCopy.pop_front();
				continue;
			}
			game_state_tracker().SetAttribute(order, F(-1));
		}
	}
}

std::optional<int> m2g::Proxy::handle_client_command(int turn_holder_index, MAYBE const m2g::pb::ClientCommand& client_command) {
	LOG_INFO("Received command from client", turn_holder_index);
	auto turn_holder_object_id = M2G_PROXY.multiPlayerObjectIds[turn_holder_index];
	auto& turn_holder_character = M2_LEVEL.objects[turn_holder_object_id].character();

	if (_is_liquidating) {
		if (auto liquidationSuccessful = HandleActionWhileLiquidating(turn_holder_character, client_command); not liquidationSuccessful) {
			LOG_INFO("Failed to handle action while liquidating", liquidationSuccessful.error());
			pb::ServerCommand sc;
			sc.set_action_failure(liquidationSuccessful.error());
			M2_GAME.ServerThread().send_server_command(sc, turn_holder_index);
			return std::nullopt;
		}
		_is_liquidating = false; // No longer liquidating
	} else {
		// Prepare action notification
		pb::ServerCommand actionNotificationCommand;
		pb::ServerCommand::ActionNotification* actionNotification = actionNotificationCommand.mutable_action_notification();
		actionNotification->set_player_index(turn_holder_index);
		auto moneySpentIfSuccessful = HandleActionWhileNotLiquidating(turn_holder_character, client_command, *actionNotification);
		if (not moneySpentIfSuccessful) {
			LOG_INFO("Failed to handle action", moneySpentIfSuccessful.error());
			pb::ServerCommand sc;
			sc.set_action_failure(moneySpentIfSuccessful.error());
			M2_GAME.ServerThread().send_server_command(sc, turn_holder_index);
			return std::nullopt;
		}

		LOG_DEBUG("Sending action notification to clients");
		if (turn_holder_index != 0) {
			// For server, do not send the command, execute it right away
			display_action_notification(actionNotificationCommand.action_notification());
		}
		for (int i = 1; i < M2_GAME.ServerThread().client_count(); ++i) {
			if (i != turn_holder_index) {
				M2_GAME.ServerThread().send_server_command(actionNotificationCommand, i);
			}
		}

		// Record spent money
		if (not _played_players.empty() && _played_players.back().first == turn_holder_index) {
			_played_players.back().second += *moneySpentIfSuccessful;
		} else {
			_played_players.emplace_back(turn_holder_index, *moneySpentIfSuccessful);
		}

		// Save spent money to game state tracker so that it's shared among clients
		for (int i = 0; i < M2_GAME.TotalPlayerCount(); ++i) {
			auto money_spent_by_player_enum = static_cast<pb::ResourceType>(pb::MONEY_SPENT_BY_PLAYER_0 + i);
			game_state_tracker().SetResource(money_spent_by_player_enum, 0.0f);
		}
		for (auto [player_index, spent_money] : _played_players) {
			auto money_spent_by_player_enum = static_cast<pb::ResourceType>(pb::MONEY_SPENT_BY_PLAYER_0 + player_index);
			game_state_tracker().SetResource(money_spent_by_player_enum, m2::F(spent_money));
		}
	}

	// Determine next player
	const auto card_count = total_card_count();
	decltype(prepare_next_round()) liquidation_necessary;
	if (_waiting_players.empty() && card_count == 0) {
		LOG_INFO("No cards left in the game");
		if (M2G_PROXY.is_canal_era()) {
			liquidation_necessary = prepare_railroad_era();
		} else {
			LOG_INFO("Ending game");
			score_links_and_remove_roads();
			score_sold_factories_and_remove_obsolete();
		}
	} else if (_is_first_turn) {
		// In first turn, the players have odd number of cards.
		// That's the difference between the first turn and following last turns.

		// Give card to player
		auto card = _draw_deck.back();
		_draw_deck.pop_back();
		game_state_tracker().SetResource(pb::DRAW_DECK_SIZE, m2::F(_draw_deck.size()));
		turn_holder_character.AddNamedItem(M2_GAME.GetNamedItem(card));

		// Check if first turn finished for all players
		if (_waiting_players.empty()) {
			LOG_INFO("First turn ended");
			_is_first_turn = false;
			game_state_tracker().SetResource(pb::IS_LAST_ACTION_OF_PLAYER, 0.0f);
			liquidation_necessary = prepare_next_round();
		} else {
			// Otherwise, just fetch the next player from _waiting_players
			LOG_INFO("Switch to next player");
		}
	} else if (card_count % 2) { // && not _is_first_turn
		// If there are odd number of cards, the turn holder does not change
		// Push to the front of the waiting players, so that it's popped first below.
		_waiting_players.push_front(turn_holder_index);
		game_state_tracker().SetResource(pb::IS_LAST_ACTION_OF_PLAYER, 1.0f);
		LOG_INFO("Continuing with the same player");
	} else { // not is_first_turn() && (card_count % 2) == 0
		// If there are even number of cards, the turn holder changes. Give cards to player
		while (not _draw_deck.empty() && PlayerCardCount(turn_holder_character) < 8) {
			auto card = _draw_deck.back();
			_draw_deck.pop_back();
			game_state_tracker().SetResource(pb::DRAW_DECK_SIZE, m2::F(_draw_deck.size()));
			turn_holder_character.AddNamedItem(M2_GAME.GetNamedItem(card));
		}

		game_state_tracker().SetResource(pb::IS_LAST_ACTION_OF_PLAYER, 0.0f);

		// Try to prepare the next round.
		if (_waiting_players.empty()) {
			liquidation_necessary = prepare_next_round();
		} else {
			// Otherwise, just fetch the next player from _waiting_players
			LOG_INFO("Switch to next player");
		}
	}

	// Now that the player order is determined, fill the game state tracker with the order
	{
		auto playedPlayersCopy = _played_players;
		auto waitingPlayersCopy = _waiting_players;
		// If only the first action is played, the last player of playedPlayers should be the same as the first player of waitingPlayers
		if (not playedPlayersCopy.empty() && not waitingPlayersCopy.empty() && playedPlayersCopy.back().first == waitingPlayersCopy.front()) {
			playedPlayersCopy.pop_back();
		}

		for (auto order = pb::FIRST_PLAYER_INDEX; order <= pb::FORTH_PLAYER_INDEX;
				order = static_cast<pb::AttributeType>(I(order) + 1)) {
			if (not playedPlayersCopy.empty()) {
				game_state_tracker().SetAttribute(order, F(playedPlayersCopy.front().first));
				playedPlayersCopy.pop_front();
				continue;
			}
			if (not waitingPlayersCopy.empty()) {
				game_state_tracker().SetAttribute(order, F(waitingPlayersCopy.front()));
				waitingPlayersCopy.pop_front();
				continue;
			}
			game_state_tracker().SetAttribute(order, F(-1));
		}
	}

	int next_turn_holder;
	if (liquidation_necessary) {
		_is_liquidating = true; // Set the liquidation state so that client commands are handled properly
		LOG_INFO("Sending liquidation command to player", liquidation_necessary->first);
		M2_GAME.ServerThread().send_server_command(liquidation_necessary->second, liquidation_necessary->first);
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
	if (server_command.has_action_failure()) {
		// If we have received this command, we must be showing the "WaitingForServerUpdate" screen, waiting for a response from the server
		if (const auto* semiBlockingUiPanel = M2_LEVEL.SemiBlockingUiPanel(); semiBlockingUiPanel && semiBlockingUiPanel->Name() == "WaitingForServerUpdate") {
			M2_LEVEL.DismissSemiBlockingUiPanel();
			// Show failure screen
			auto blueprint = UiPanelBlueprint{
				.name = "ActionFailure",
				.w = 60, .h = 60,
				.background_color = {0, 0, 0, 255},
				.widgets = {
					UiWidgetBlueprint{
						.x = 5, .y = 5, .w = 50, .h = 40,
						.border_width = 0,
						.variant = widget::TextBlueprint{
							.text = server_command.action_failure(),
							.horizontal_alignment = TextHorizontalAlignment::LEFT,
							.vertical_alignment = TextVerticalAlignment::TOP,
							.wrapped_font_size_in_units = 3.0f
						}
					},
					UiWidgetBlueprint{
						.x = 5, .y = 50, .w = 50, .h = 5,
						.variant = widget::TextBlueprint{
							.text = "Close",
							.wrapped_font_size_in_units = 3.0f,
							.onAction = [](MAYBE const widget::Text& self) -> UiAction {
								return MakeReturnAction();
							}
						}
					}
				}
			};
			// Play sound
			M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_NOTIFICATION_SOUND], m2::AudioManager::ONCE);
			// Show screen
			M2_LEVEL.ShowSemiBlockingUiPanel(RectF{0.3f, 0.3f, 0.4f, 0.4f}, std::make_unique<m2::UiPanelBlueprint>(blueprint));
		} else {
			LOG_WARN("Expected to find the WaitingForServerUpdate panel, but not found");
		}
	} else if (server_command.has_liquidate_assets_for_loan()) {
		LOG_INFO("Received liquidate command, beginning liquidation journey");
		auto money_to_be_paid = server_command.liquidate_assets_for_loan();
		M2G_PROXY.main_journeys.emplace(std::in_place_type<LiquidationJourney>, money_to_be_paid);
	} else if (server_command.has_action_notification()) {
		display_action_notification(server_command.action_notification());
	} else if (server_command.has_canal_era_result()) {
		LOG_INFO("Received CanalEraResult command");
		display_canal_era_result(server_command.canal_era_result());
	} else {
		throw M2_ERROR("Unsupported server command");
	}
}

void m2g::Proxy::post_server_update(m2::SequenceNo, const bool shutdown) {
	// If we have received a server update, we might have taken an action and showing the "WaitingForServerUpdate" screen.
	if (const auto* semiBlockingUiPanel = M2_LEVEL.SemiBlockingUiPanel(); semiBlockingUiPanel && semiBlockingUiPanel->Name() == "WaitingForServerUpdate") {
		M2_LEVEL.DismissSemiBlockingUiPanel();
	} else {
		// Don't remove other type of screen. ServerUpdate is received when others take an action too
	}

	// Delete the custom hud
	if (custom_hud_panel) {
		LOG_DEBUG("Hiding top HUD");
		M2_LEVEL.RemoveCustomNonblockingUiPanel(*custom_hud_panel);
		custom_hud_panel = std::nullopt;
	}

	LOG_DEBUG("Refreshing status bar");
	M2_LEVEL.RemoveCustomNonblockingUiPanel(_status_bar_panel);
	_status_bar_panel = M2_LEVEL.AddCustomNonblockingUiPanel(
			std::make_unique<m2::UiPanelBlueprint>(generate_status_bar_blueprint()),
			status_bar_window_ratio());

	if (cards_panel) {
		LOG_DEBUG("Refreshing cards panel");
		M2_LEVEL.RemoveCustomNonblockingUiPanel(*M2G_PROXY.cards_panel);
		M2G_PROXY.cards_panel = M2_LEVEL.AddCustomNonblockingUiPanel(
				std::make_unique<m2::UiPanelBlueprint>(generate_cards_window("Cards")),
				cards_panel_ratio());
	}

	if (shutdown) {
		LOG_INFO("Game is shutting down");
		display_game_result();
	} else {
		// Enable/disable buttons
		if (M2_GAME.IsOurTurn()) {
			enable_action_buttons();
		} else {
			disable_action_buttons();
		}
	}
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
		M2_GAME.FindBot(receiver_index).queue_client_command(cc);
	}
}

void m2g::Proxy::bot_handle_server_command(MAYBE const m2g::pb::ServerCommand& server_command, MAYBE int receiver_index) {
	INFO_FN();
}

void m2g::Proxy::post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type) {
	// Store the positions of the merchants
	if (is_merchant_location(sprite_type)) {
		auto merchant_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 2.0f, 2.0f};
		merchant_positions[sprite_type] = std::make_tuple(obj.position, merchant_cell_rect, obj.id());
		LOG_DEBUG("Merchant position", m2g::pb::SpriteType_Name(sprite_type), merchant_cell_rect);
	}
	// Store the positions of the industries build locations
	else if (is_industry_location(sprite_type)) {
		// Verify that ppm of the industry tiles are double of the sprite sheet
		const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));
		if (sprite.Ppm() != sprite.Sheet().Pb().ppm()) {
			throw M2_ERROR("Sprite ppm mismatch");
		}
		auto industry_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 2.0f, 2.0f};
		industry_positions[sprite_type] = std::make_tuple(obj.position, industry_cell_rect, obj.id());
		LOG_DEBUG("Industry position", m2g::pb::SpriteType_Name(sprite_type), industry_cell_rect);
	}
	// Store the positions of the connection locations
	else if (is_canal(sprite_type) || is_railroad(sprite_type)) {
		m2::RectF connection_cell_rect = m2::RectF{obj.position.x - 0.5f, obj.position.y - 0.5f, 1.0f, 1.0f};
		// Different canal or railroad backgrounds have different offsets
		const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(sprite_type));
		auto original_type = sprite.OriginalType(); // Connection sprites are duplicate of another
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

m2::void_expected m2g::Proxy::LoadForegroundObjectFromLevelBlueprint(MAYBE m2::Object& obj) {
	return m2::make_unexpected("Invalid object type");
}

m2::void_expected m2g::Proxy::init_server_update_fg_object(m2::Object& obj, const std::vector<m2g::pb::ItemType>& items,
	MAYBE const std::vector<m2::pb::Resource>& resources) {
	switch (obj.object_type()) {
		case pb::FACTORY: {
			auto city = std::ranges::find_if(items, is_city);
			auto industry_tile = std::ranges::find_if(items, is_industry_tile);
			if (city != items.end() && industry_tile != items.end()) {
				return InitFactory(obj, *city, *industry_tile);
			} else {
				return m2::make_unexpected("Unable to find city or industry tile of the object received from the server");
			}
		}
		case pb::ROAD: {
			if (auto connection = connection_on_position(obj.position)) {
				return InitRoad(obj, *connection);
			} else {
				return m2::make_unexpected("Unable to find connection from object location");
			}
		}
		default:
			return m2::make_unexpected("Invalid object type");
	}
}

m2g::Proxy& m2g::Proxy::get_instance() {
	return dynamic_cast<m2g::Proxy&>(M2_GAME.Proxy());
}

void m2g::Proxy::main_journey_deleter() {
	get_instance().main_journeys.reset();
}

unsigned m2g::Proxy::player_index(m2::Id id) const {
	auto it = std::find(multiPlayerObjectIds.begin(), multiPlayerObjectIds.end(), id);
	if (it != multiPlayerObjectIds.end()) {
		return U(std::distance(multiPlayerObjectIds.begin(), it));
	} else {
		throw M2_ERROR("Invalid player ID");
	}
}

m2::Character& m2g::Proxy::game_state_tracker() const {
	return M2_LEVEL.objects[_game_state_tracker_id].character();
}
int m2g::Proxy::total_card_count() const {
	const auto player_card_lists = M2G_PROXY.multiPlayerObjectIds
		| std::views::transform(m2::to_object_of_id)
		| std::views::transform(m2::to_character_of_object)
		| std::views::transform(m2::GenerateNamedItemTypesFilter({pb::ITEM_CATEGORY_CITY_CARD, pb::ITEM_CATEGORY_INDUSTRY_CARD, pb::ITEM_CATEGORY_WILD_CARD}));
	const auto card_count = std::accumulate(player_card_lists.begin(), player_card_lists.end(), 0, [](const int sum, const std::vector<Card>& card_list) { return sum + I(card_list.size()); });
	return card_count + m2::iround(game_state_tracker().GetResource(pb::DRAW_DECK_SIZE));
}
bool m2g::Proxy::is_last_action_of_player() const {
	return m2::is_equal(game_state_tracker().GetResource(pb::IS_LAST_ACTION_OF_PLAYER), 1.0f, 0.001f);
}
bool m2g::Proxy::is_canal_era() const {
	return m2::is_equal(game_state_tracker().GetResource(pb::IS_RAILROAD_ERA), 0.0f, 0.001f);
}
bool m2g::Proxy::is_railroad_era() const {
	return m2::is_equal(game_state_tracker().GetResource(pb::IS_RAILROAD_ERA), 1.0f, 0.001f);
}
int m2g::Proxy::market_coal_count() const {
	return m2::iround(game_state_tracker().GetResource(m2g::pb::COAL_CUBE_COUNT));
}
int m2g::Proxy::market_iron_count() const {
	return m2::iround(game_state_tracker().GetResource(m2g::pb::IRON_CUBE_COUNT));
}
int m2g::Proxy::market_coal_cost(int coal_count) const {
	auto current_coal_count = m2::iround(game_state_tracker().GetResource(m2g::pb::COAL_CUBE_COUNT));
	return CalculateCost(COAL_MARKET_CAPACITY, current_coal_count, coal_count);
}
int m2g::Proxy::market_iron_cost(int iron_count) const {
	auto current_iron_count = m2::iround(game_state_tracker().GetResource(m2g::pb::IRON_CUBE_COUNT));
	return CalculateCost(IRON_MARKET_CAPACITY, current_iron_count, iron_count);
}
int m2g::Proxy::player_spent_money(int player_index) const {
	auto money_spent_by_player_enum = static_cast<pb::ResourceType>(pb::MONEY_SPENT_BY_PLAYER_0 + player_index);
	return m2::iround(game_state_tracker().GetResource(money_spent_by_player_enum));
}
std::pair<int,int> m2g::Proxy::market_coal_revenue(int count) const {
	auto current_coal_count = m2::iround(game_state_tracker().GetResource(m2g::pb::COAL_CUBE_COUNT));
	return CalculateRevenue(COAL_MARKET_CAPACITY, current_coal_count, count);
}
std::pair<int,int> m2g::Proxy::market_iron_revenue(int count) const {
	auto current_iron_count = m2::iround(game_state_tracker().GetResource(m2g::pb::IRON_CUBE_COUNT));
	return CalculateRevenue(IRON_MARKET_CAPACITY, current_iron_count, count);
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
		auto* button = M2_LEVEL.LeftHud()->find_first_widget_by_name<m2::widget::Text>(button_name);
		button->enabled = true;
	}
}
void m2g::Proxy::disable_action_buttons() {
	for (const auto& button_name : action_button_names) {
		auto* button = M2_LEVEL.LeftHud()->find_first_widget_by_name<m2::widget::Text>(button_name);
		button->enabled = false;
	}
}
void m2g::Proxy::SendClientCommandAndWaitForServerUpdate(const pb::ClientCommand& cc) {
	auto blueprint = m2::UiPanelBlueprint{
			.name = "WaitingForServerUpdate",
			.w = 1, .h = 1,
			.background_color = {0, 0, 0, 255},
			.onCreate = [&cc](UiPanel&) {
				M2_GAME.QueueClientCommand(cc);
			},
			.widgets = {
				m2::UiWidgetBlueprint{
					.x = 0, .y = 0, .w = 1, .h = 1,
					.border_width = 0,
					.variant = m2::widget::TextBlueprint{
						.text = "Waiting for server...",
						.wrapped_font_size_in_units = 0.1f
					}
				},
			}};
	M2_LEVEL.ShowSemiBlockingUiPanel(m2::RectF{0.3f, 0.3f, 0.4f, 0.4f}, std::make_unique<m2::UiPanelBlueprint>(blueprint));
}

std::optional<std::pair<m2g::Proxy::PlayerIndex, m2g::pb::ServerCommand>> m2g::Proxy::prepare_next_round() {
	LOG_INFO("Preparing next round");

	// First, before preparing the next round, check if liquidation is necessary.
	if (auto liquidation = IsLiquidationNecessary()) {
		LOG_INFO("Liquidation is necessary");
		// Prepare the ServerCommand and return
		pb::ServerCommand sc;
		sc.set_liquidate_assets_for_loan(liquidation->second);
		return std::make_pair(liquidation->first, sc);
	}

	// Gain incomes
	for (const auto playerId : M2G_PROXY.multiPlayerObjectIds) {
		// Lookup player
		auto& player_character = M2_LEVEL.objects[playerId].character();
		const auto incomePoints = m2::iround(player_character.GetAttribute(pb::INCOME_POINTS));
		const auto incomeLevel = IncomeLevelFromIncomePoints(incomePoints);
		const auto playerMoney = m2::iround(player_character.GetResource(pb::MONEY));
		LOG_DEBUG("Player gained money", incomeLevel);
		const auto newPlayerMoney = playerMoney + incomeLevel;
		if (newPlayerMoney < 0) {
			LOG_INFO("Player doesn't have enough money to pay its loan, they'll lose victory points", newPlayerMoney);
			player_character.AddResource(pb::VICTORY_POINTS, m2::F(newPlayerMoney));
		}
		player_character.SetResource(pb::MONEY, m2::F(std::clamp(newPlayerMoney, 0, INT32_MAX)));
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
		auto it = std::remove_if(_played_players.begin(), _played_players.end(),
				[player_index](const auto& pair) {
					return pair.first == player_index;
				});
		_played_players.erase(it);

		_waiting_players.emplace_back(player_index);
	}

	// Liquidation not necessary
	return std::nullopt;
}

m2g::Proxy::LiquidationDetails m2g::Proxy::prepare_railroad_era() {
	if (auto isLiquidationNecessary = prepare_next_round()) {
		return isLiquidationNecessary;
	}

	LOG_INFO("Preparing railroad era");
	score_links_and_remove_roads();
	score_sold_factories_and_remove_obsolete();

	// Send canal era results
	pb::ServerCommand canal_era_result_command;
	std::ranges::for_each(M2G_PROXY.multiPlayerObjectIds
		| std::views::transform(m2::to_object_of_id)
		| std::views::transform(m2::to_character_of_object),
		[&](const m2::Character& human_player) {
			canal_era_result_command.mutable_canal_era_result()->add_victory_points(
				m2::iround(human_player.GetResource(pb::VICTORY_POINTS)));
		});
	LOG_INFO("Sending CanalEraResult to clients");
	M2_GAME.ServerThread().send_server_command(canal_era_result_command, -1);

	// Reset merchant beer
	for (const auto& merchantObjId: merchant_object_ids | std::views::values) {
		auto& merchantChr = M2_LEVEL.objects[merchantObjId].character();
		if (merchantChr.HasItem(pb::ITEM_CATEGORY_MERCHANT_LICENSE)) {
			merchantChr.SetResource(pb::BEER_BARREL_COUNT, 1.0f);
		}
	}

	// Shuffle the draw deck
	auto draw_deck = PrepareDrawDeck(M2_GAME.ServerThread().client_count());
	Give8CardsToEachPlayer(draw_deck);
	_draw_deck = std::move(draw_deck);
	game_state_tracker().SetResource(pb::DRAW_DECK_SIZE, m2::F(_draw_deck.size()));

	// Give roads to players
	const auto& road_item = M2_GAME.GetNamedItem(pb::ROAD_TILE);
	auto road_possession_limit = m2::zround(road_item.GetAttribute(pb::POSSESSION_LIMIT));
	std::ranges::for_each(M2G_PROXY.multiPlayerObjectIds
		| std::views::transform(m2::to_object_of_id)
		| std::views::transform(m2::to_character_of_object),
		[&](m2::Character& human_player) {
			while (human_player.CountItem(pb::ROAD_TILE) < road_possession_limit) {
				human_player.AddNamedItem(road_item);
			}
		});

	game_state_tracker().SetResource(pb::IS_RAILROAD_ERA, 1.0f);
	game_state_tracker().SetResource(pb::IS_LAST_ACTION_OF_PLAYER, 0.0f);
	LOG_INFO("Switch to railroad era and next player");
	return std::nullopt;
}

void m2g::Proxy::buy_coal_from_market() {
	game_state_tracker().RemoveResource(pb::COAL_CUBE_COUNT, 1.0f);
}

void m2g::Proxy::buy_iron_from_market() {
	game_state_tracker().RemoveResource(pb::IRON_CUBE_COUNT, 1.0f);
}

void m2g::Proxy::sell_coal_to_market(int count) {
	game_state_tracker().AddResource(pb::COAL_CUBE_COUNT, m2::F(count));
}

void m2g::Proxy::sell_iron_to_market(int count) {
	game_state_tracker().AddResource(pb::IRON_CUBE_COUNT, m2::F(count));
}
