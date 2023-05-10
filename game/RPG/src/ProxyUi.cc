#include <m2/Proxy.h>
#include <rpg/Context.h>
#include <m2/Game.h>
#include <m2/M2.h>

using namespace m2::ui;

static Blueprint::Widget::Variant resume_button = Blueprint::Widget::Text{
	.initial_text = "Resume",
	.action_callback = []() {
		LOG_DEBUG("Resume button pressed");
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_R
};
static Blueprint::Widget::Variant quit_button = Blueprint::Widget::Text{
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
			.variant = resume_button
		},
		Blueprint::Widget{
			.x = 75, .y = 78, .w = 10, .h = 6,
			.border_width_px = 1,
			.variant = quit_button
		}
	}
};

static Blueprint::Widget::Variant hp_label = Blueprint::Widget::Text{
	.initial_text = "HP"
};
static Blueprint::Widget::Variant hp_progress_bar = Blueprint::Widget::ProgressBar{
		.initial_progress = 1.0f,
		.bar_color = SDL_Color{255, 0, 0, 255},
		.update_callback = []() -> float {
			if (LEVEL.player()) {
				return LEVEL.player()->character().get_resource(m2g::pb::RESOURCE_HP);
			}
			return 0.0f;
		}
};
static Blueprint::Widget::Variant dash_label = Blueprint::Widget::Text{
	.initial_text = "DASH"
};
static Blueprint::Widget::Variant dash_progress_bar = Blueprint::Widget::ProgressBar{
	.initial_progress = 1.0f,
	.bar_color = SDL_Color{255, 255, 0, 255},
	.update_callback = []() {
		if (LEVEL.player()) {
			// Check if player has DASH capability
			if (LEVEL.player()->character().has_item(m2g::pb::ITEM_REUSABLE_DASH_2S)) {
				float counter = LEVEL.player()->character().get_resource(m2g::pb::RESOURCE_DASH_ENERGY);
				float cooldown = 2.0f;
				counter = (cooldown <= counter) ? cooldown : counter;
				return counter / cooldown;
			}
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
			.border_width_px = 0,
			.variant = hp_label
		},
		Blueprint::Widget{
			.x = 4, .y = 52, .w = 11, .h = 2,
			.variant = hp_progress_bar
		},
		Blueprint::Widget{
			.x = 4, .y = 56, .w = 11, .h = 2,
			.border_width_px = 0,
			.variant = dash_label
		},
		Blueprint::Widget{
			.x = 4, .y = 58, .w = 11, .h = 2,
			.border_width_px = 1,
			.variant = dash_progress_bar
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
