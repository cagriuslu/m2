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

const m2::ui::Blueprint* rpg::Context::main_menu() const {
	_main_menu = m2::ui::Blueprint{
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = SDL_Color{20, 20, 20, 255}
	};

	auto level_jsons = m2::list_files(GAME.game_resource_dir / "Levels", ".json");
	LOG_INFO("Adding level buttons", level_jsons.size());
	for (unsigned i = 0; i < level_jsons.size(); ++i) {
		const auto& level_json = level_jsons[i];
		auto level_name = level_json.stem().string();

		bool level_completed = progress.level_completion_times().contains(level_name);

		auto row = i / 6; // 6 rows
		auto col = i % 8; // 8 columns

		unsigned x_padding = 26, y_padding = 17;
		unsigned x_button_width = 10, y_button_width = 6;
		unsigned button_gap = 4;
		_main_menu.widgets.emplace_back(m2::ui::Blueprint::Widget{
			.x = x_padding + col * (x_button_width + button_gap),
			.y = y_padding + row * (y_button_width + button_gap),
			.w = x_button_width, .h = y_button_width,
			.border_width_px = 1,
			.variant = m2::ui::Blueprint::Widget::Text{
				.initial_text = level_completed ? level_name : '*' + level_name + '*',
				.action_callback = [=]() {
					auto success = GAME.load_single_player(level_json, level_name);
					m2_throw_failure_as_error(success);
					GAME.audio_manager->play(&GAME.get_song(m2g::pb::SONG_MAIN_THEME), m2::AudioManager::PlayPolicy::LOOP, 0.5f);
					return m2::ui::Action::RETURN;
				}
			}
		});
	}

	LOG_DEBUG("Adding quit button");
	_main_menu.widgets.emplace_back(m2::ui::Blueprint::Widget{
		.x = 75, .y = 78, .w = 10, .h = 6,
		.border_width_px = 1,
		.variant = m2::ui::Blueprint::Widget::Text{
			.initial_text = "Quit",
			.action_callback = []() {
				return m2::ui::Action::QUIT;
			},
			.kb_shortcut = SDL_SCANCODE_Q
		}
	});

	return &_main_menu;
}

const m2::ui::Blueprint* rpg::Context::you_died_menu() const {
	_you_died_menu = m2::ui::Blueprint{
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = SDL_Color{20, 20, 20, 255}
	};

	auto lb_path = LEVEL.path();
	if (lb_path) {
		_you_died_menu.widgets.emplace_back(m2::ui::Blueprint::Widget{
			.x = 70, .y = 30, .w = 20, .h = 6,
			.border_width_px = 1,
			.variant = m2::ui::Blueprint::Widget::Text{
				.initial_text = "Retry",
				.action_callback = [=]() -> m2::ui::Action {
					auto success = GAME.load_single_player(*lb_path, LEVEL.name());
					m2_throw_failure_as_error(success);
					return m2::ui::Action::RETURN;
				}
			}
		});
	}

	_you_died_menu.widgets.emplace_back(m2::ui::Blueprint::Widget{
		.x = 70, .y = 42, .w = 20, .h = 6,
		.border_width_px = 1,
		.variant = m2::ui::Blueprint::Widget::Text{
			.initial_text = "Main Menu",
			.action_callback = [&]() {
				return m2::ui::execute_blocking(&_main_menu);
			}
		}
	});

	_you_died_menu.widgets.emplace_back(m2::ui::Blueprint::Widget{
		.x = 70, .y = 54, .w = 20, .h = 6,
		.border_width_px = 1,
		.variant = m2::ui::Blueprint::Widget::Text{
			.initial_text = "Quit",
			.action_callback = []() {
				return m2::ui::Action::QUIT;
			}
		}
	});

	return &_you_died_menu;
}
