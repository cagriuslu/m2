#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/protobuf/Detail.h>
#include <m2g/Proxy.h>
#include <m2g_SpriteType.pb.h>

#include "m2/game/Noise.h"
#include <m2/FileSystem.h>

using namespace m2;
using namespace m2::widget;
using namespace m2g;
using namespace m2g::pb;

namespace {
	auto quit_button_action = [](MAYBE const widget::Text &self) {
		return MakeQuitAction();
	};
}

static TextBlueprint entry_variant_1 = {
		.text = "RANDOM LEVEL",
		.kb_shortcut = SDL_SCANCODE_R,
		.on_action = [](MAYBE const widget::Text &self) {
			auto expect_lb = m2::pb::json_file_to_message<m2::pb::Level>(m2::resource_path() / "game/MINE/levels/sp000.json");
			if (!expect_lb) {
				throw M2_ERROR("Unable to load level");
			}
			m2::pb::Level lb = std::move(*expect_lb);

			for (int y = 0; y < 100; ++y) {
				for (int x = 0; x < 100; ++x) {
					if (y < 5 && x < 5)
						continue;

					auto noise = m2::perlin({x * 0.4f, y * 0.4f}, 1.0f);
					auto st = (noise < 0.5f) ? GRASSLAND_DIRT_1 : DUNGEON_COAL_1;

					m2::pb::mutable_get_or_create(lb.mutable_background_layers(), 0);
					auto *sprite_array = m2::pb::mutable_get_or_create(
							lb.mutable_background_layers(0)->mutable_background_rows(), y);
					auto *sprite = m2::pb::mutable_get_or_create(sprite_array->mutable_items(), x);
					*sprite = st;
				}
			}

			M2_GAME.LoadSinglePlayer(lb);
			return MakeReturnAction(); // TODO Return value
		}
};
static TextBlueprint entry_variant_2 = {
		.text = "NEW GAME",
		.kb_shortcut = SDL_SCANCODE_N,
		.on_action = [](MAYBE const widget::Text &self) {
			auto success = M2_GAME.LoadSinglePlayer(m2::resource_path() / "game/MINE/levels/sp000.json");
			if (!success) {
				LOG_ERROR("Unable to load level", success.error());
			}
			return MakeReturnAction(); // TODO Return value
		}
};
static TextBlueprint entry_variant_3 = {
		.text = "QUIT",
		.kb_shortcut = SDL_SCANCODE_Q,
		.on_action = quit_button_action
};
const UiPanelBlueprint main_menu_blueprint = {
	.name = "MainMenu",
		.w = 100, .h = 100,
		.background_color = {20, 20, 20, 255},
		.widgets = {
				UiWidgetBlueprint{
						.x = 45, .y = 15, .w = 10, .h = 10,
						.variant = entry_variant_1
				},
				UiWidgetBlueprint{
						.x = 45, .y = 35, .w = 10, .h = 10,
						.variant = entry_variant_2
				},
				UiWidgetBlueprint{
						.x = 45, .y = 55, .w = 10, .h = 10,
						.variant = entry_variant_3
				}
		}
};

static TextBlueprint pause_variant_1 = {
		.text = "RESUME GAME",
		.horizontal_alignment = TextHorizontalAlignment::CENTER,
		.kb_shortcut = SDL_SCANCODE_R,
		.on_action = [](MAYBE const widget::Text &self) {
			return MakeReturnAction(); // TODO Return value
		}
};
static TextBlueprint pause_variant_2 = {
		.text = "QUIT",
		.kb_shortcut = SDL_SCANCODE_Q,
		.on_action = quit_button_action,
};
const UiPanelBlueprint pause_menu_blueprint = {
	.name = "PauseMenu",
		.w = 100, .h = 100,
		.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
		.widgets = {
				UiWidgetBlueprint{
						.x = 45, .y = 35, .w = 10, .h = 10,
						.variant = pause_variant_1
				},
				UiWidgetBlueprint{
						.x = 45, .y = 55, .w = 10, .h = 10,
						.variant = pause_variant_2
				}
		}
};

const UiPanelBlueprint left_hud_blueprint = {
	.name = "LeftHud",
	.w = 19, .h = 72,
	.widgets = {}
};

const UiPanelBlueprint right_hud_blueprint = {
	.name = "RightHud",
	.w = 19, .h = 72,
	.widgets = {}
};

const m2::UiPanelBlueprint* m2g::Proxy::MainMenuBlueprint() {
	return &main_menu_blueprint;
}

const m2::UiPanelBlueprint* m2g::Proxy::PauseMenuBlueprint() {
	return &pause_menu_blueprint;
}

const m2::UiPanelBlueprint* m2g::Proxy::LeftHudBlueprint() {
	return &left_hud_blueprint;
}

const m2::UiPanelBlueprint* m2g::Proxy::RightHudBlueprint() {
	return &right_hud_blueprint;
}
