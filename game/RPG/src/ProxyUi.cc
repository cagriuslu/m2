#include <m2/Proxy.h>
#include <rpg/Context.h>
#include <m2/FileSystem.h>
#include <m2/Game.h>
#include <m2/M2.h>

using namespace m2::ui;

namespace {
	Action quit_button_action() {
		LOG_INFO("Quit button pressed");
		return Action::QUIT;
	}
}

static Blueprint::Widget::Variant pause_menu_variant_1 = Blueprint::Widget::Text{
	.initial_text = "Resume",
	.action_callback = []() {
		LOG_DEBUG("Resume button pressed");
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_R
};
static Blueprint::Widget::Variant pause_menu_variant_2 = Blueprint::Widget::Text{
	.initial_text = "Quit",
	.action_callback = quit_button_action,
	.kb_shortcut = SDL_SCANCODE_Q
};
const Blueprint pause_menu_blueprint = {
	.w = 160, .h = 90,
	.border_width_px = 0,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.widgets = {
		Blueprint::Widget{
			.x = 75, .y = 42, .w = 10, .h = 6,
			.border_width_px = 1,
			.variant = pause_menu_variant_1
		},
		Blueprint::Widget{
			.x = 75, .y = 78, .w = 10, .h = 6,
			.border_width_px = 1,
			.variant = pause_menu_variant_2
		}
	}
};

static Blueprint::Widget::Variant left_hud_variant_1 = Blueprint::Widget::Text{
	.initial_text = "HP"
};
static Blueprint::Widget::Variant left_hud_variant_2 = Blueprint::Widget::Text{
	.initial_text = "100",
	.update_callback = []() -> std::pair<Action,std::string> {
		// Lookup player
		if (LEVEL.player_id) {
			float hp = LEVEL.player()->character().get_resource(m2g::pb::RESOURCE_HP);
			return {Action::CONTINUE, std::to_string((int)round(hp * 100.0f))};
		} else {
			return {Action::CONTINUE, {}};
		}
	}
};
static Blueprint::Widget::Variant left_hud_variant_3 = Blueprint::Widget::Text{
	.initial_text = "DASH"
};
static Blueprint::Widget::Variant left_hud_variant_4 = Blueprint::Widget::ProgressBar{
	.initial_progress = 1.0f,
	.bar_color = SDL_Color{255, 255, 0, 255},
	.update_callback = []() {
		if (LEVEL.player_id) {
			float counter = LEVEL.player()->character().get_resource(m2g::pb::RESOURCE_DASH_ENERGY);
			float cooldown = 2.0f;
			counter = (cooldown <= counter) ? cooldown : counter;
			return counter / cooldown;
		}
		return 0.0f;
	}
};
const Blueprint left_hud_blueprint = {
	.w = 19, .h = 72,
	.border_width_px = 2,
	.widgets = {
		Blueprint::Widget{
			.x = 4, .y = 50, .w = 11, .h = 2,
			.variant = left_hud_variant_1
		},
		Blueprint::Widget{
			.x = 4, .y = 52, .w = 11, .h = 2,
			.variant = left_hud_variant_2
		},
		Blueprint::Widget{
			.x = 4, .y = 54, .w = 11, .h = 2,
			.variant = left_hud_variant_3
		},
		Blueprint::Widget{
			.x = 4, .y = 56, .w = 11, .h = 2,
			.border_width_px = 1,
			.variant = left_hud_variant_4
		}
	}
};

const Blueprint right_hud_blueprint = {
	.w = 19, .h = 72,
	.border_width_px = 2
};

const m2::ui::Blueprint* m2g::ui::main_menu() {
	// Generate main menu
	auto& context = rpg::Context::get_instance();
	context.main_menu_blueprint = m2::ui::Blueprint{
		.w = 160, .h = 90,
		.border_width_px = 0,
		.background_color = SDL_Color{20, 20, 20, 255}
	};

	auto level_jsons = m2::list_files(GAME.game_resource_dir / "Levels", ".json");
	LOG_INFO("Adding level buttons", level_jsons.size());
	for (unsigned i = 0; i < level_jsons.size(); ++i) {
		const auto& level_json = level_jsons[i];
		auto level_name = level_json.stem().string();

		bool level_completed = context.progress.level_completion_times().contains(level_name);

		auto row = i / 6; // 6 rows
		auto col = i % 8; // 8 columns

		unsigned x_padding = 26, y_padding = 17;
		unsigned x_button_width = 10, y_button_width = 6;
		unsigned button_gap = 4;
		context.main_menu_blueprint.widgets.emplace_back(m2::ui::Blueprint::Widget{
			.x = x_padding + col * (x_button_width + button_gap),
			.y = y_padding + row * (y_button_width + button_gap),
			.w = x_button_width, .h = y_button_width,
			.border_width_px = 1,
			.variant = Blueprint::Widget::Text{
				.initial_text = level_completed ? level_name : '*' + level_name + '*',
				.action_callback = [=]() {
					LOG_INFO("Loading level...", level_json);
					auto success = GAME.load_single_player(level_json, level_name);
					m2_throw_failure_as_error(success);
					LOG_INFO("Level loaded");
					GAME.audio_manager->play(&GAME.get_song(m2g::pb::SONG_MAIN_THEME), m2::AudioManager::PlayPolicy::LOOP, 0.5f);
					return Action::RETURN;
				}
			}
		});
	}

	LOG_DEBUG("Adding quit button");
	context.main_menu_blueprint.widgets.emplace_back(m2::ui::Blueprint::Widget{
		.x = 75, .y = 78, .w = 10, .h = 6,
		.border_width_px = 1,
		.variant = Blueprint::Widget::Text{
			.initial_text = "Quit",
			.action_callback = quit_button_action,
			.kb_shortcut = SDL_SCANCODE_Q
		}
	});

	return &context.main_menu_blueprint;
}
const m2::ui::Blueprint* m2g::ui::pause_menu() {
	return &pause_menu_blueprint;
}
const m2::ui::Blueprint* m2g::ui::left_hud() {
	return &left_hud_blueprint;
}
const m2::ui::Blueprint* m2g::ui::right_hud() {
	return &right_hud_blueprint;
}
