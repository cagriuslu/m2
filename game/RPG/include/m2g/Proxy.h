#pragma once
#include <m2/Proxy.h>
#include <Enemy.pb.h>
#include <Progress.pb.h>

namespace m2g {
	class Proxy : public m2::Proxy {
	   public:
		const std::string game_name = "RPG";
		const bool camera_is_listener = true;

		void load_resources();

		const m2::ui::Blueprint* main_menu();
		const m2::ui::Blueprint* pause_menu();
		const m2::ui::Blueprint* left_hud();
		const m2::ui::Blueprint* right_hud();

		void post_single_player_level_init(const std::string& name, const m2::pb::Level& level);
		m2::void_expected init_fg_object(m2::Object& obj);
		m2::Group* create_group(m2g::pb::GroupType group_type);

		// Game specific
	   public:
		rpg::pb::Enemies enemies;
		unsigned alive_enemy_count{};
		const rpg::pb::Enemy* get_enemy(m2g::pb::ObjectType object_type) const;

		std::filesystem::path progress_file_path;
		rpg::pb::Progress progress;
		void save_progress() const;

		m2::ui::Blueprint _main_menu;
		const m2::ui::Blueprint* generate_main_menu();

		m2::ui::Blueprint _right_hud;
		const m2::ui::Blueprint* generate_right_hud();
		void set_ammo_display_state(bool enabled);

		m2::ui::Blueprint _you_died_menu;
		const m2::ui::Blueprint* you_died_menu();
	};
}
