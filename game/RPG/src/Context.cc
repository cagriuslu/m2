#include <rpg/Context.h>
#include <m2/Game.h>
#include <m2/protobuf/Detail.h>

rpg::Context::Context() {
	// Load enemies
	auto expect_enemies = m2::protobuf::json_file_to_message<pb::Enemies>(GAME.game_resource_dir / "Enemies.json");
	m2_throw_failure_as_error(expect_enemies);
	enemies = *expect_enemies;
	// Load progress
	progress_file_path = GAME.game_resource_dir / "Progress.json";
	auto expect_progress = m2::protobuf::json_file_to_message<rpg::pb::Progress>(progress_file_path);
	if (expect_progress) {
		progress.CopyFrom(*expect_progress);
	} else {
		LOG_INFO("Unable to load Progress file");
	}
}

rpg::Context& rpg::Context::get_instance() {
	// Context is stored in GAME
	return *reinterpret_cast<Context*>(GAME.context);
}

const rpg::pb::Enemy* rpg::Context::get_enemy(m2g::pb::ObjectType object_type) const {
	for (const auto& enemy : enemies.enemies()) {
		if (enemy.object_type() == object_type) {
			return &enemy;
		}
	}
	return nullptr;
}

void rpg::Context::save_progress() const {
	m2::protobuf::message_to_json_file(progress, progress_file_path);
}
