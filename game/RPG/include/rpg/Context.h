#pragma once
#include <m2/Ui.h>
#include <Enemy.pb.h>
#include <Progress.pb.h>
#include <filesystem>

namespace rpg {
	struct Context {
		Context();
		static Context& get_instance();

		pb::Enemies enemies;
		unsigned alive_enemy_count{};
		const pb::Enemy* get_enemy(m2g::pb::ObjectType object_type) const;

		std::filesystem::path progress_file_path;
		pb::Progress progress;
		void save_progress() const;

		m2::ui::Blueprint _main_menu;
		const m2::ui::Blueprint* main_menu();

		m2::ui::Blueprint _right_hud;
		const m2::ui::Blueprint* right_hud();
		void set_ammo_display_state(bool enabled);

		m2::ui::Blueprint _you_died_menu;
		const m2::ui::Blueprint* you_died_menu();
	};
}
