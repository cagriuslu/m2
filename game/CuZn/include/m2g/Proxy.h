#pragma once

#include <cuzn/detail/Market.h>
#include <cuzn/Detail.h>
#include <cuzn/detail/Build.h>
#include <m2/Proxy.h>
#include <cuzn/journeys/BuildJourney.h>
#include <cuzn/journeys/NetworkJourney.h>
#include <m2/Cache.h>

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
			unsigned turn_holder_index, const pb::ClientCommand& client_command);
		void post_tile_create(m2::Object& obj, pb::SpriteType sprite_type);
		m2::void_expected init_level_blueprint_fg_object(m2::Object& obj);
		m2::void_expected init_server_update_fg_object(m2::Object&,
			const std::vector<m2g::pb::ItemType>&, const std::vector<m2::pb::Resource>&);

	private:
		m2::Id _market_object_id{};
		// Only the host initializes these fields
		std::optional<cuzn::Market<cuzn::COAL_MARKET_CAPACITY>> _coal_market;
		std::optional<cuzn::Market<cuzn::IRON_MARKET_CAPACITY>> _iron_market;

	public:
		// Once the level is created, these should not be modified.
		std::vector<SDL_Color> player_colors;
		std::unordered_map<pb::SpriteType, m2::VecI> merchant_positions;
		std::unordered_map<m2::VecI, pb::SpriteType, m2::VecIHash> position_merchants;
		std::unordered_map<pb::SpriteType, m2::Id> merchant_object_ids;  // Contains only active merchants
		std::unordered_map<pb::SpriteType, std::pair<m2::VecF,m2::RectF>> industry_positions; // Exact position and cell rectangle
		std::unordered_map<pb::SpriteType, m2::RectF> network_positions;

		std::optional<std::variant<cuzn::BuildJourney, cuzn::NetworkJourney>> user_journey;
		static void user_journey_deleter();

		[[nodiscard]] unsigned player_index(m2::Id id) const;
	};
}  // namespace m2g
