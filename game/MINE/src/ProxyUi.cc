#include <m2/Proxy.h>
#include <m2/Game.h>
#include <m2/Noise.h>
#include <m2/protobuf/Utils.h>
#include <SpriteType.pb.h>

using namespace m2::ui;
using namespace m2g;
using namespace m2g::pb;

namespace {
	auto quit_button_action = []() {
		return Action::QUIT;
	};
}

static Blueprint::Widget::Variant entry_variant_1 = Blueprint::Widget::Text{
		.initial_text = "RANDOM LEVEL",
		.action_callback = []() {
			auto expect_lb = m2::proto::json_file_to_message<m2::pb::Level>("resource/game/MINE/levels/sp000.json");
			if (!expect_lb) {
				throw M2ERROR("Unable to load level");
			}
			m2::pb::Level lb = std::move(*expect_lb);

			for (int y = 0; y < 100; ++y) {
				for (int x = 0; x < 100; ++x) {
					if (y < 5 && x < 5)
						continue;

					auto noise = m2::perlin({x * 0.4f, y * 0.4f}, 1.0f);
					auto st = (noise < 0.5f) ? GRASSLAND_DIRT_1 : DUNGEON_COAL_1;

					auto* sprite_array = m2::proto::mutable_get_or_create(lb.mutable_background_rows(), y);
					auto* sprite = m2::proto::mutable_get_or_create(sprite_array->mutable_items(), x);
					*sprite = st;
				}
			}

			GAME.load_single_player(lb);
			return Action::RETURN;
		},
		.kb_shortcut = SDL_SCANCODE_R
};
static Blueprint::Widget::Variant entry_variant_2 = Blueprint::Widget::Text{
	.initial_text = "NEW GAME",
	.action_callback = []() {
		auto success = GAME.load_single_player("resource/game/MINE/levels/sp000.json");
		if (!success) {
			LOG_ERROR("Unable to load level", success.error());
		}
		return Action::RETURN;
	},
	.kb_shortcut = SDL_SCANCODE_N
};
static Blueprint::Widget::Variant entry_variant_3 = Blueprint::Widget::Text{
	.initial_text = "QUIT",
	.action_callback = quit_button_action,
	.kb_shortcut = SDL_SCANCODE_Q
};
const Blueprint main_menu_blueprint = {
	.w = 100, .h = 100,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		Blueprint::Widget{
			.x = 45, .y = 15, .w = 10, .h = 10,
			.border_width_px = 1,
			.padding_width_px = 5,
			.variant = entry_variant_1
		},
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
const Blueprint pause_menu_blueprint = {
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

const Blueprint left_hud_blueprint = {
	.w = 19, .h = 72,
	.border_width_px = 2
};

const Blueprint right_hud_blueprint = {
	.w = 19, .h = 72,
	.border_width_px = 2
};

const m2::ui::Blueprint* m2g::ui::main_menu() {
	return &main_menu_blueprint;
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
