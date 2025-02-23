#include <m2g/Proxy.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/M2.h>
#include <m2/ui/widget/ProgressBar.h>

using namespace m2;

static widget::TextBlueprint resume_button = {
		.text = "Resume",
		.kb_shortcut = SDL_SCANCODE_R,
		.onAction = [](MAYBE const widget::Text &self) {
			LOG_DEBUG("Resume button pressed");
			return MakeReturnAction(); // TODO Return value
		}
};
static widget::TextBlueprint quit_button = {
		.text = "Quit",
		.kb_shortcut = SDL_SCANCODE_Q,
		.onAction = [](MAYBE const widget::Text &self) {
			return MakeQuitAction();
		}
};
const UiPanelBlueprint pause_menu_blueprint = {
	.name = "PauseMenu",
	.w = 160, .h = 90,
	.border_width = 0,
	.background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
	.widgets = {
		UiWidgetBlueprint{
			.x = 75, .y = 42, .w = 10, .h = 6,
			.variant = resume_button
		},
		UiWidgetBlueprint{
			.x = 75, .y = 78, .w = 10, .h = 6,
			.variant = quit_button
		}
	}
};

static widget::TextBlueprint hp_label = {
		.text = "HP"
};
static widget::ProgressBarBlueprint hp_progress_bar = {
		.initial_progress = 1.0f,
		.bar_color = SDL_Color{255, 0, 0, 255},
		.onUpdate = [](widget::ProgressBar& self) {
			if (M2_LEVEL.Player()) {
				self.SetProgress(M2_PLAYER.character().GetResource(m2g::pb::RESOURCE_HP));
			}
			self.SetProgress(0.0f);
		}
};
static widget::TextBlueprint dash_label = {
		.text = "DASH COOLDOWN"
};
static widget::ProgressBarBlueprint dash_progress_bar = {
		.initial_progress = 1.0f,
		.bar_color = SDL_Color{255, 255, 0, 255},
		.onUpdate = [](widget::ProgressBar& self) {
			if (M2_LEVEL.Player()) {
				// Check if player has DASH capability
				if (M2_PLAYER.character().HasItem(m2g::pb::ITEM_REUSABLE_DASH_2S)) {
					float counter = M2_PLAYER.character().GetResource(m2g::pb::RESOURCE_DASH_ENERGY);
					float cooldown = 2.0f;
					counter = (cooldown <= counter) ? cooldown : counter;
					self.SetProgress(counter / cooldown);
				}
			}
			self.SetProgress(0.0f);
		}
};
const UiPanelBlueprint left_hud_blueprint = {
		.name = "LeftHud",
		.w = 19, .h = 72,
		.background_color = {0, 0, 0, 255},
		.widgets = {
				UiWidgetBlueprint{
						.x = 2, .y = 60, .w = 15, .h = 2,
						.border_width = 0,
						.variant = hp_label
				},
				UiWidgetBlueprint{
						.x = 2, .y = 62, .w = 15, .h = 2,
						.variant = hp_progress_bar
				},
				UiWidgetBlueprint{
						.x = 2, .y = 66, .w = 15, .h = 2,
						.border_width = 0,
						.variant = dash_label
				},
				UiWidgetBlueprint{
						.x = 2, .y = 68, .w = 15, .h = 2,
						.variant = dash_progress_bar
				}
		}
};

const m2::UiPanelBlueprint* m2g::Proxy::MainMenuBlueprint() {
	return M2G_PROXY.generate_main_menu();
}

const m2::UiPanelBlueprint* m2g::Proxy::PauseMenuBlueprint() {
	return &pause_menu_blueprint;
}

const m2::UiPanelBlueprint* m2g::Proxy::LeftHudBlueprint() {
	return &left_hud_blueprint;
}

const m2::UiPanelBlueprint* m2g::Proxy::RightHudBlueprint() {
	return M2G_PROXY.generate_right_hud();
}
