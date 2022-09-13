#include <m2g/ui/UI.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/Game.hh>
#include <rpg/LevelBlueprint.h>
#include <m2/M2.h>
#include <rpg/object/Player.h>
#include <m2g/SpriteBlueprint.h>

using namespace m2::ui;

namespace {
	auto quit_button_action = []() {
		return Action::QUIT;
	};
}

static WidgetBlueprint::Variant entry_variant_0 = wdg::TextBlueprint{
	.initial_text = "Select save slot:"
};
static WidgetBlueprint::Variant entry_variant_1 = wdg::ImageBlueprint{
	.initial_sprite_key = "main.floppy_disk",
	.action_callback = []() {
		fprintf(stderr, "Pressed button\n");
		return Action::CONTINUE;
	},
	.kb_shortcut = SDL_SCANCODE_1
};
static WidgetBlueprint::Variant entry_variant_2 = wdg::TextBlueprint{
	.initial_text = "NEW GAME",
	.action_callback = []() {
		GAME.load_level("resource/game/RPG/levels/sp000.json");
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_N
};
static WidgetBlueprint::Variant entry_variant_3 = wdg::TextBlueprint{
	.initial_text = "QUIT",
	.action_callback = quit_button_action,
	.kb_shortcut = SDL_SCANCODE_Q
};
const UIBlueprint m2g::ui::entry = {
	.w = 100, .h = 100,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 45, .y = 0, .w = 10, .h = 10,
			.background_color = {20, 20, 20, 255},
			.variant = entry_variant_0
		},
		WidgetBlueprint{
			.x = 45, .y = 10, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = entry_variant_1
		},
		WidgetBlueprint{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = entry_variant_2
		},
		WidgetBlueprint{
			.x = 45, .y = 55, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = entry_variant_3
		}
	}
};

static WidgetBlueprint::Variant pause_variant_1 = wdg::TextBlueprint{
	.initial_text = "RESUME GAME",
	.alignment = TextAlignment::CENTER,
	.action_callback = []() {
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_R
};
static WidgetBlueprint::Variant pause_variant_2 = wdg::TextBlueprint{
	.initial_text = "QUIT",
	.action_callback = quit_button_action,
	.kb_shortcut = SDL_SCANCODE_Q
};
const UIBlueprint m2g::ui::pause = {
	.w = 100, .h = 100,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.widgets = {
		WidgetBlueprint{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = pause_variant_1
		},
		WidgetBlueprint{
			.x = 45, .y = 55, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = pause_variant_2
		}
	}
};

static WidgetBlueprint::Variant left_hud_variant_1 = wdg::TextBlueprint{
	.initial_text = "HP"
};
static WidgetBlueprint::Variant left_hud_variant_2 = wdg::TextBlueprint{
	.initial_text = "100",
	.update_callback = []() -> std::pair<Action,std::string> {
		// Lookup player
		static m2::ID player_id = 0;
		static float* hp = nullptr;
		if (player_id != GAME.playerId) {
			player_id = GAME.playerId;
			hp = &GAME.player()->defense().hp;
		}
		// Read HP
		if (hp) {
			return {Action::CONTINUE, std::to_string((int)round(*hp))};
		} else {
			return {Action::CONTINUE, {}};
		}
	}
};
static WidgetBlueprint::Variant left_hud_variant_3 = wdg::TextBlueprint{
	.initial_text = "DASH"
};
static WidgetBlueprint::Variant left_hud_variant_4 = wdg::ProgressBarBlueprint{
	.initial_progress = 1.0f,
	.bar_color = SDL_Color{255, 255, 0, 255},
	.update_callback = []() {
		// Lookup player
		static m2::ID player_id = 0;
		static float* counter = nullptr;
		static float cooldown = 0.0f;
		if (player_id != GAME.playerId) {
			player_id = GAME.playerId;
			auto* impl = dynamic_cast<obj::Player*>(GAME.player()->impl.get());
			counter = &impl->char_state.dash_cooldown_counter_s;
			cooldown = impl->char_state.blueprint->dash_cooldown_s;
		}
		// Read cooldown counter
		if (counter) {
			return *counter / cooldown;
		} else {
			return 0.0f;
		}
	}
};
const UIBlueprint m2g::ui::left_hud = {
	.w = 19, .h = 72,
	.border_width_px = 2,
	.widgets = {
		WidgetBlueprint{
			.x = 4, .y = 50, .w = 11, .h = 2,
			.variant = left_hud_variant_1
		},
		WidgetBlueprint{
			.x = 4, .y = 52, .w = 11, .h = 2,
			.variant = left_hud_variant_2
		},
		WidgetBlueprint{
			.x = 4, .y = 54, .w = 11, .h = 2,
			.variant = left_hud_variant_3
		},
		WidgetBlueprint{
			.x = 4, .y = 56, .w = 11, .h = 2,
			.border_width_px = 1,
			.variant = left_hud_variant_4
		}
	}
};

const UIBlueprint m2g::ui::right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 2
};
