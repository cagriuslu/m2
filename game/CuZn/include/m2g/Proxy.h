#pragma once
#include <m2/Proxy.h>

namespace m2g {
	class Proxy : public m2::Proxy {
		std::unordered_map<m2::VecI, m2g::pb::SpriteType, m2::VecIHash> _merchant_positions;

	   public:
		const std::string game_name = "CuZn";

		const m2::ui::Blueprint* main_menu();
		const m2::ui::Blueprint* pause_menu();
		const m2::ui::Blueprint* left_hud();
		const m2::ui::Blueprint* right_hud();

		void post_multi_player_level_init(const std::string& name, const m2::pb::Level& level);
		void multi_player_level_host_populate(const std::string& name, const m2::pb::Level& level);

		std::optional<int> handle_client_command(unsigned turn_holder_index, const m2g::pb::ClientCommand& client_command);
		void post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type);
		m2::void_expected init_fg_object(m2::Object& obj);

	   private:
		std::vector<m2g::pb::ItemType> prepare_merchant_license_list(int client_count);
		std::vector<m2g::pb::SpriteType> pick_merchants(int client_count);
		std::vector<m2g::pb::ItemType> prepare_draw_deck(int client_count);
	};
}  // namespace m2g
