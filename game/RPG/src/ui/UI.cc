#include <m2g/ui/UI.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/Game.hh>
#include <rpg/LevelBlueprint.h>
#include <m2/M2.h>

using namespace m2::ui;

auto no_string = []() {
	return std::make_pair(Action::CONTINUE, std::optional<std::string>{});
};
auto quit_button_action = []() {
	return Action::QUIT;
};

static ElementBlueprint::ElementBlueprintVariant entry_variant_1 = element::TextBlueprint{
	.initial_text = "NEW GAME",
	.update_callback = no_string,
	.action_callback = []() {
		Game_Level_Load(&lvl::sp_000);
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_N
};
static ElementBlueprint::ElementBlueprintVariant entry_variant_2 = element::TextBlueprint{
	.initial_text = "QUIT",
	.update_callback = no_string,
	.action_callback = quit_button_action,
	.kb_shortcut = SDL_SCANCODE_Q
};
const UIBlueprint m2g::ui::entry = {
	.w = 100, .h = 100,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.elements = {
		ElementBlueprint{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = entry_variant_1
		},
		ElementBlueprint{
			.x = 45, .y = 55, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = entry_variant_2
		}
	}
};

static ElementBlueprint::ElementBlueprintVariant pause_variant_1 = element::TextBlueprint{
	.initial_text = "RESUME_GAME",
	.update_callback = no_string,
	.action_callback = []() {
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_R
};
static ElementBlueprint::ElementBlueprintVariant pause_variant_2 = element::TextBlueprint{
	.initial_text = "QUIT",
	.update_callback = no_string,
	.action_callback = quit_button_action,
	.kb_shortcut = SDL_SCANCODE_Q
};
const UIBlueprint m2g::ui::pause = {
	.w = 100, .h = 100,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.elements = {
		ElementBlueprint{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = pause_variant_1
		},
		ElementBlueprint{
			.x = 45, .y = 55, .w = 10, .h = 10,
			.border_width_px = 1,
			.variant = pause_variant_2
		}
	}
};

static ElementBlueprint::ElementBlueprintVariant left_hud_variant_1 = element::TextBlueprint{
	.initial_text = "HP",
	.update_callback = no_string
};
static ElementBlueprint::ElementBlueprintVariant left_hud_variant_2 = element::TextBlueprint{
	.initial_text = "100",
	.update_callback = []() {
		// Lookup player's health
		auto *player = GAME.objects.get(GAME.playerId);
		if (player) {
			return std::make_pair(Action::CONTINUE, std::make_optional(m2::round_string(player->defense().hp)));
		} else {
			return std::make_pair(Action::CONTINUE, std::optional<std::string>{});
		}
	}
};
const UIBlueprint m2g::ui::left_hud = {
	.w = 19, .h = 72,
	.border_width_px = 2,
	.elements = {
		ElementBlueprint{
			.x = 4, .y = 50, .w = 11, .h = 2,
			.variant = left_hud_variant_1
		},
		ElementBlueprint{
			.x = 4, .y = 52, .w = 11, .h = 2,
			.variant = left_hud_variant_2
		}
	}
};

const UIBlueprint m2g::ui::right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 2
};
