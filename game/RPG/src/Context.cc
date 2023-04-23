#include <rpg/Context.h>
#include <m2/Game.h>
#include <m2/protobuf/Detail.h>

rpg::Context::Context() {
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

void rpg::Context::save_progress() const {
	m2::protobuf::message_to_json_file(progress, progress_file_path);
}
