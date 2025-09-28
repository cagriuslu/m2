#pragma once

#include <m2/Proxy.h>
#include <cuzn/journeys/BuildJourney.h>
#include <cuzn/journeys/NetworkJourney.h>
#include <cuzn/journeys/DevelopJourney.h>
#include <cuzn/journeys/SellJourney.h>
#include <cuzn/journeys/LiquidationJourney.h>
#include <cuzn/journeys/subjourneys/POISelectionJourney.h>
#include <m2/game/Graph.h>
#include <list>

namespace m2g {
	class Proxy : public m2::Proxy {
	public:
		static Proxy& get_instance(); // TODO remove

		const std::string gameIdentifier = "CuZn";
		const std::string gameFriendlyName = "OpenBrass";
		const int gameAspectRatioMul = 3;
		const int gameAspectRatioDiv = 2;
		const float defaultGameHeightM = 30.0f;
		const std::string defaultFontPath = "fonts/Roboto_Mono/RobotoMono-VariableFont_wght.ttf";

		const m2::UiPanelBlueprint* MainMenuBlueprint();
		const m2::UiPanelBlueprint* PauseMenuBlueprint();
		const m2::UiPanelBlueprint* LeftHudBlueprint();
		const m2::UiPanelBlueprint* RightHudBlueprint();

		void postTurnBasedLevelClientInit(const std::string& name, const m2::pb::Level& level);
		void turnBasedServerPopulate(const std::string& name, const m2::pb::Level& level);
		std::optional<int> handle_client_command(int turn_holder_index, const pb::TurnBasedClientCommand& client_command);
		void handle_server_command(const pb::TurnBasedServerCommand& server_command);
		void post_server_update(m2::network::SequenceNo, bool shutdown);
		void bot_handle_server_update(const m2::pb::TurnBasedServerUpdate& server_update);
		void bot_handle_server_command(const m2g::pb::TurnBasedServerCommand& server_command, int receiver_index);
		void post_tile_create(m2::Object& obj, pb::SpriteType sprite_type);
		m2::void_expected init_server_update_fg_object(m2::Object&, const m2::VecF& position, const std::vector<m2g::pb::ItemType>&, const std::vector<m2::pb::Resource>&);

	private:
		m2::Id _game_state_tracker_id{};
		std::list<m2::UiPanel>::iterator _status_bar_panel;

	public:
		std::optional<std::list<m2::UiPanel>::iterator> custom_hud_panel, cards_panel, actionNotificationPanel;

		// Once the level is created, these should not be modified.

		std::vector<m2::RGB> player_colors;
		std::unordered_map<pb::SpriteType, std::tuple<m2::VecF,m2::RectF,m2::ObjectId>> merchant_positions;
		/// Contains all merchants since they are all "active". Active merchants are different from merchants with a
		/// license. Only select merchants have a license.
		std::unordered_map<pb::SpriteType, m2::Id> merchant_object_ids;
		std::unordered_map<pb::SpriteType, std::tuple<m2::VecF,m2::RectF,m2::ObjectId>> industry_positions; // Exact position, cell rectangle, tile object ID
		std::unordered_map<pb::SpriteType, std::tuple<m2::VecF,m2::RectF,m2::ObjectId>> connection_positions;
		m2::Graph<City> available_connections_graph; // Nodes are City // TODO instead of holding onto this object, maybe recreate it by looking at sprites each time it's needed

		// User journeys

		std::optional<std::variant<BuildJourney, SellJourney, NetworkJourney, DevelopJourney, LiquidationJourney>> main_journeys;
		static void main_journey_deleter();

		// Accessors

		[[nodiscard]] unsigned player_index(m2::Id id) const;
		[[nodiscard]] m2::Character& game_state_tracker() const;
		[[nodiscard]] int total_card_count() const;
		[[nodiscard]] bool is_last_action_of_player() const;
		[[nodiscard]] bool is_canal_era() const;
		[[nodiscard]] bool is_railroad_era() const;
		[[nodiscard]] int market_coal_count() const;
		[[nodiscard]] int market_iron_count() const;
		[[nodiscard]] int market_coal_cost(int coal_count) const; // Query cost of buying coal from the market
		[[nodiscard]] int market_iron_cost(int iron_count) const; // Query cost of buying iron from the market
		[[nodiscard]] int player_spent_money(int player_index) const;
		// first: number of items that can be sold, second: revenue of selling
		[[nodiscard]] std::pair<int,int> market_coal_revenue(int count) const; // Query revenue of selling coal to the market
		[[nodiscard]] std::pair<int,int> market_iron_revenue(int count) const; // Query revenue of selling iron to the market
		[[nodiscard]] std::set<m2::ObjectId> object_ids_of_industry_location_bg_tiles(const std::set<IndustryLocation>&) const;
		[[nodiscard]] std::set<m2::ObjectId> object_ids_of_connection_bg_tiles(const std::set<Connection>&) const;

		// Modifiers

		void enable_action_buttons();
		void disable_action_buttons();
		void SendClientCommandAndWaitForServerUpdate(const pb::TurnBasedClientCommand& cc);

		// Server only fields

		std::vector<Card> _draw_deck;
		bool _is_first_turn{true};
		/// Liquidation is a special state where a player needs to sell some of its factories to pay back their loan.
		/// In this state, the current state holder is the player that needs to sell its factories.
		bool _is_liquidating{};

		using PlayerIndex = int;
		using SpentMoney = int;
		/// List of players waiting their turn. Front of the list is the next player
		std::list<PlayerIndex> _waiting_players;
		/// List of players that have already taken one or both of their actions. Front of the list is the player that went the first.
		std::list<std::pair<PlayerIndex, SpentMoney>> _played_players;

		using LiquidationDetails = std::optional<std::pair<PlayerIndex, pb::TurnBasedServerCommand>>;
		/// If non-null, the index of the player that should liquidate their assets is returned.
		LiquidationDetails prepare_next_round();
		LiquidationDetails prepare_railroad_era();

		void buy_coal_from_market();
		void buy_iron_from_market();
		void sell_coal_to_market(int count);
		void sell_iron_to_market(int count);
	};
}  // namespace m2g
