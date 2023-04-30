#include <m2/Proxy.h>
#include <rpg/Context.h>
#include <m2/Game.h>
#include <m2/M2.h>

using namespace m2::ui;

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
	.action_callback = []() {
		return m2::ui::Action::QUIT;
	},
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
	return rpg::Context::get_instance().main_menu();
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
