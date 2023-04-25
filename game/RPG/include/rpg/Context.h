#ifndef RPG_CONTEXT_H
#define RPG_CONTEXT_H

#include <m2/Ui.h>
#include <Enemy.pb.h>
#include <Progress.pb.h>
#include <filesystem>

namespace rpg {
	struct Context {
		pb::Enemies enemies;

		std::filesystem::path progress_file_path;
		pb::Progress progress;

		m2::ui::Blueprint main_menu_blueprint; // Cache

		unsigned alive_enemy_count{};

		Context();
		static Context& get_instance();

		const pb::Enemy* get_enemy(m2g::pb::ObjectType object_type) const;

		void save_progress() const;
	};
}

#endif //RPG_CONTEXT_H
