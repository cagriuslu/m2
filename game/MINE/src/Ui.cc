#include <m2g/Ui.h>
#include <m2/Game.hh>
#include <m2/M2.h>
#include <m2g/Object.h>

using namespace m2::ui;

namespace {
	auto quit_button_action = []() {
		return Action::QUIT;
	};
}

static Blueprint::Widget::Variant entry_variant_2 = Blueprint::Widget::Text{
	.initial_text = "NEW GAME",
	.action_callback = []() {
		GAME.load_level("resource/game/MINE/levels/sp000.json");
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_N
};
static Blueprint::Widget::Variant entry_variant_3 = Blueprint::Widget::Text{
	.initial_text = "QUIT",
	.action_callback = quit_button_action,
	.kb_shortcut = SDL_SCANCODE_Q
};
const Blueprint m2g::ui::entry = {
	.w = 100, .h = 100,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		Blueprint::Widget{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = entry_variant_2
		},
		Blueprint::Widget{
			.x = 45, .y = 55, .w = 10, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = entry_variant_3
		}
	}
};

static Blueprint::Widget::Variant pause_variant_1 = Blueprint::Widget::Text{
	.initial_text = "RESUME GAME",
	.alignment = TextAlignment::CENTER,
	.action_callback = []() {
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_R
};
static Blueprint::Widget::Variant pause_variant_2 = Blueprint::Widget::Text{
	.initial_text = "QUIT",
	.action_callback = quit_button_action,
	.kb_shortcut = SDL_SCANCODE_Q
};
const Blueprint m2g::ui::pause = {
	.w = 100, .h = 100,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.widgets = {
		Blueprint::Widget{
			.x = 45, .y = 35, .w = 10, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = pause_variant_1
		},
		Blueprint::Widget{
			.x = 45, .y = 55, .w = 10, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = pause_variant_2
		}
	}
};

const Blueprint m2g::ui::left_hud = {
	.w = 19, .h = 72,
	.border_width_px = 2
};

const Blueprint m2g::ui::right_hud = {
	.w = 19, .h = 72,
	.border_width_px = 2
};
