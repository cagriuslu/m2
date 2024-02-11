#pragma once
#include <m2/Proxy.h>

namespace m2g {
	class Proxy : public m2::Proxy {
	   public:
		const std::string game_name = "CuZn";

		const m2::ui::Blueprint* main_menu();
		const m2::ui::Blueprint* pause_menu();
		const m2::ui::Blueprint* left_hud();
		const m2::ui::Blueprint* right_hud();

		void multi_player_level_host_populate(MAYBE const std::string& name, MAYBE const m2::pb::Level& level);

		std::optional<int> handle_client_command(unsigned turn_holder_index, const m2g::pb::ClientCommand& client_command);
		m2::void_expected init_fg_object(MAYBE m2::Object& obj);
	};
}  // namespace m2g
