#pragma once

#include <cuzn/Detail.h>
#include <cuzn/detail/Build.h>
#include <m2/Proxy.h>
#include <cuzn/journeys/BuildJourney.h>
#include <cuzn/journeys/NetworkJourney.h>
#include <cuzn/journeys/DevelopJourney.h>
#include <cuzn/journeys/SellJourney.h>
#include <cuzn/journeys/LiquidationJourney.h>
#include <m2/Cache.h>
#include <m2/game/Graph.h>

namespace m2g {
	class Proxy : public m2::Proxy {
	public:
		static Proxy& get_instance();

		const std::string game_name = "CuZn";

		const m2::ui::Blueprint* main_menu();
		const m2::ui::Blueprint* pause_menu();
		const m2::ui::Blueprint* left_hud();
		const m2::ui::Blueprint* right_hud();

		void post_multi_player_level_client_init(const std::string& name, const m2::pb::Level& level);
		void multi_player_level_server_populate(const std::string& name, const m2::pb::Level& level);
		std::optional<int> handle_client_command(
			int turn_holder_index, const pb::ClientCommand& client_command);
		void handle_server_command(const pb::ServerCommand& server_command);
		void post_server_update(const m2::pb::ServerUpdate& server_update);
		void post_tile_create(m2::Object& obj, pb::SpriteType sprite_type);
		m2::void_expected init_level_blueprint_fg_object(m2::Object& obj);
		m2::void_expected init_server_update_fg_object(m2::Object&,
			const std::vector<m2g::pb::ItemType>&, const std::vector<m2::pb::Resource>&);

	private:
		m2::Id _game_state_tracker_id{};

	public:
		// Once the level is created, these should not be modified.
		std::vector<m2::RGB> player_colors;
		std::unordered_map<pb::SpriteType, std::pair<m2::VecF,m2::RectF>> merchant_positions;
		std::unordered_map<pb::SpriteType, m2::Id> merchant_object_ids;  // Contains only active merchants
		std::unordered_map<pb::SpriteType, std::tuple<m2::VecF,m2::RectF,m2::ObjectId>> industry_positions; // Exact position, cell rectangle, tile object ID
		std::unordered_map<pb::SpriteType, std::pair<m2::VecF,m2::RectF>> connection_positions;
		m2::Graph available_connections_graph; // Nodes are City (m2g::pb::ItemType) // TODO instead of holding onto this object, maybe recreate it by looking at sprites each time it's needed

		// User journeys
		std::optional<std::variant<BuildJourney, NetworkJourney, DevelopJourney, SellJourney, LiquidationJourney>> user_journey;
		static void user_journey_deleter();

		// Accessors
		[[nodiscard]] unsigned player_index(m2::Id id) const;
		[[nodiscard]] m2::Character& game_state_tracker() const;
		[[nodiscard]] bool is_first_turn() const;
		[[nodiscard]] bool is_canal_era() const;
		[[nodiscard]] bool is_railroad_era() const;
		[[nodiscard]] std::set<m2::ObjectId> object_ids_of_industry_location_bg_tiles(const std::set<IndustryLocation>&) const;

		// Modifiers
		void enable_action_buttons();
		void disable_action_buttons();

		// Server only fields
		std::vector<Card> _draw_deck;
		using PlayerIndex = int;
		std::list<PlayerIndex> _waiting_players; // Front of the list is the next player
		using SpentMoney = int;
		std::list<std::pair<PlayerIndex, SpentMoney>> _played_players; // Front of the list played first
		std::optional<std::pair<PlayerIndex, m2g::pb::ServerCommand>> prepare_next_round(); // Returns the index of the player that should liquidate assets
	};
}  // namespace m2g
