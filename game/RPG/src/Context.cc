#include <rpg/Context.h>
#include <m2/Game.h>
#include <m2/protobuf/Detail.h>

rpg::Context::Context() {
	// Load enemies
	auto expect_enemies = m2::protobuf::json_file_to_message<pb::Enemies>(GAME.resource_dir / "Enemies.json");
	m2_throw_failure_as_error(expect_enemies);
	enemies = *expect_enemies;
	// Load progress
	progress_file_path = GAME.resource_dir / "Progress.json";
	auto expect_progress = m2::protobuf::json_file_to_message<rpg::pb::Progress>(progress_file_path);
	if (expect_progress) {
		progress.CopyFrom(*expect_progress);
	} else {
		LOG_INFO("Unable to load Progress file");
	}
}

rpg::Context &rpg::Context::get_instance() {
	// Context is stored in GAME
	return *reinterpret_cast<Context *>(GAME.context);
}

const rpg::pb::Enemy *rpg::Context::get_enemy(m2g::pb::ObjectType object_type) const {
	for (const auto &enemy: enemies.enemies()) {
		if (enemy.object_type() == object_type) {
			return &enemy;
		}
	}
	return nullptr;
}

void rpg::Context::save_progress() const {
	m2::protobuf::message_to_json_file(progress, progress_file_path);
}

const m2::ui::Blueprint *rpg::Context::main_menu() {
	_main_menu = m2::ui::Blueprint{
			.w = 160, .h = 90,
			.border_width_px = 0,
			.background_color = SDL_Color{20, 20, 20, 255}
	};

	auto level_jsons = m2::list_files(GAME.resource_dir / "levels", ".json");
	for (int i = 0; i < (ssize_t) level_jsons.size(); ++i) {
		const auto &level_json = level_jsons[i];
		auto level_name = level_json.stem().string();
		LOG_INFO("Adding level button", level_name);

		bool level_completed = progress.level_completion_times().contains(level_name);
		auto level_display_name = level_completed ? level_name : '*' + level_name + '*';

		auto row = i / 6; // 6 rows
		auto col = i % 8; // 8 columns

		int x_padding = 26, y_padding = 17;
		int x_button_width = 10, y_button_width = 6;
		int button_gap = 4;
		_main_menu.widgets.emplace_back(m2::ui::WidgetBlueprint{
				.x = x_padding + col * (x_button_width + button_gap),
				.y = y_padding + row * (y_button_width + button_gap),
				.w = x_button_width, .h = y_button_width,
				.border_width_px = 1,
				.variant = m2::ui::widget::TextBlueprint{
						.initial_text = level_display_name,
						.on_action = [=](MAYBE const m2::ui::widget::Text &self) {
							Context::get_instance().alive_enemy_count = 0;
							auto success = GAME.load_single_player(level_json, level_name);
							m2_throw_failure_as_error(success);
							GAME.audio_manager->play(&GAME.get_song(m2g::pb::SONG_MAIN_THEME),
									m2::AudioManager::PlayPolicy::LOOP, 0.5f);
							return m2::ui::Action::RETURN;
						}
				}
		});
	}

	LOG_DEBUG("Adding quit button");
	_main_menu.widgets.emplace_back(m2::ui::WidgetBlueprint{
			.x = 75, .y = 78, .w = 10, .h = 6,
			.border_width_px = 1,
			.variant = m2::ui::widget::TextBlueprint{
					.initial_text = "Quit",
					.kb_shortcut = SDL_SCANCODE_Q,
					.on_action = [](MAYBE const m2::ui::widget::Text &self) {
						return m2::ui::Action::QUIT;
					}
			}
	});

	return &_main_menu;
}

const m2::ui::Blueprint *rpg::Context::right_hud() {
	_right_hud = m2::ui::Blueprint{
			.w = 19, .h = 72,
			.border_width_px = 2,
			.background_color = {0, 0, 0, 255}
	};

	_right_hud.widgets.emplace_back(m2::ui::WidgetBlueprint{
			.initially_enabled = false,
			.x = 2, .y = 66, .w = 15, .h = 2,
			.border_width_px = 0,
			.variant = m2::ui::widget::TextBlueprint{
					.initial_text = "AMMO"
			}
	});
	_right_hud.widgets.emplace_back(m2::ui::WidgetBlueprint{
			.initially_enabled = false,
			.x = 2, .y = 68, .w = 15, .h = 2,
			.border_width_px = 1,
			.variant = m2::ui::widget::ProgressBarBlueprint{
					.bar_color = SDL_Color{0, 127, 255, 255},
					.on_update = [](MAYBE const m2::ui::widget::ProgressBar& self) {
						if (auto *player = LEVEL.player(); player) {
							if (auto ammo = player->character().get_resource(
										m2g::pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO); ammo != 0.0f) {
								if (auto weapon = player->character().find_items(
											m2g::pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON); weapon) {
									return ammo /
											weapon->get_acquire_benefit(m2g::pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO);
								}
							}
						}
						return 0.0f;
					}
			}
	});

	return &_right_hud;
}

void rpg::Context::set_ammo_display_state(bool enabled) {
	LEVEL.right_hud_ui_state->widgets[0]->enabled = enabled;
	LEVEL.right_hud_ui_state->widgets[1]->enabled = enabled;
}

const m2::ui::Blueprint *rpg::Context::you_died_menu() {
	_you_died_menu = m2::ui::Blueprint{
			.w = 160, .h = 90,
			.border_width_px = 0,
			.background_color = SDL_Color{20, 20, 20, 255}
	};

	auto lb_path = LEVEL.path();
	if (lb_path) {
		_you_died_menu.widgets.emplace_back(m2::ui::WidgetBlueprint{
				.x = 70, .y = 30, .w = 20, .h = 6,
				.border_width_px = 1,
				.variant = m2::ui::widget::TextBlueprint{
						.initial_text = "Retry",
						.on_action = [=](MAYBE const m2::ui::widget::Text &self) -> m2::ui::Action {
							Context::get_instance().alive_enemy_count = 0;
							auto success = GAME.load_single_player(*lb_path, LEVEL.name());
							m2_throw_failure_as_error(success);
							return m2::ui::Action::RETURN;
						}
				}
		});
	}

	_you_died_menu.widgets.emplace_back(m2::ui::WidgetBlueprint{
			.x = 70, .y = 42, .w = 20, .h = 6,
			.border_width_px = 1,
			.variant = m2::ui::widget::TextBlueprint{
					.initial_text = "Main Menu",
					.on_action = [&](MAYBE const m2::ui::widget::Text &self) {
						return m2::ui::State::create_execute_sync(&_main_menu);
					}
			}
	});

	_you_died_menu.widgets.emplace_back(m2::ui::WidgetBlueprint{
			.x = 70, .y = 54, .w = 20, .h = 6,
			.border_width_px = 1,
			.variant = m2::ui::widget::TextBlueprint{
					.initial_text = "Quit",
					.on_action = [](MAYBE const m2::ui::widget::Text &self) {
						return m2::ui::Action::QUIT;
					}
			}
	});

	return &_you_died_menu;
}
