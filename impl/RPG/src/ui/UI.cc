#include <impl/public/ui/UI.h>
#include <impl/private/ui/Callbacks.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/Game.hh>
#include <impl/private/LevelBlueprint.h>

auto no_string = []() { return std::make_pair(m2::ui::Action::CONTINUE, std::optional<std::string>{}); };
auto no_action = []() { return m2::ui::Action::CONTINUE; };
auto new_game_button_action = []() {
	Game_Level_Load(&impl::level::sp_000);
	return m2::ui::Action::RETURN;
};
auto resume_game_button_action = []() {
	return m2::ui::Action::RETURN;
};
auto quit_button_action = []() {
	return m2::ui::Action::QUIT;
};
auto hp_update = []() {
	// TODO
	return std::make_pair(m2::ui::Action::CONTINUE, std::optional<std::string>{});
};

const m2::ui::UIBlueprint impl::ui::entry = {
        .w = 100, .h = 100,
        .background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
        .elements = {
            m2::ui::ElementBlueprint{
                .x = 45, .y = 35, .w = 10, .h = 10,
                .border_width_px = 1,
                .variant = m2::ui::element::TextBlueprint{
					.initial_text = "NEW GAME",
					.update_callback = no_string,
					.action_callback = new_game_button_action,
					.kb_shortcut = SDL_SCANCODE_N
                }
            },
            m2::ui::ElementBlueprint{
                .x = 45, .y = 55, .w = 10, .h = 10,
                .border_width_px = 1,
                .variant = m2::ui::element::TextBlueprint{
					.initial_text = "QUIT",
					.update_callback = no_string,
					.action_callback = quit_button_action,
					.kb_shortcut = SDL_SCANCODE_Q
                }
            }
        }
};

const m2::ui::UIBlueprint impl::ui::pause = {
        .w = 100, .h = 100,
        .background_color = {.r = 20, .g = 20, .b = 20, .a = 255},
        .elements = {
                m2::ui::ElementBlueprint{
                        .x = 45, .y = 35, .w = 10, .h = 10,
                        .border_width_px = 1,
                        .variant = m2::ui::element::TextBlueprint{
							.initial_text = "RESUME_GAME",
							.update_callback = no_string,
							.action_callback = resume_game_button_action,
							.kb_shortcut = SDL_SCANCODE_R
                        }
                },
                m2::ui::ElementBlueprint{
                        .x = 45, .y = 55, .w = 10, .h = 10,
                        .border_width_px = 1,
                        .variant = m2::ui::element::TextBlueprint{
							.initial_text = "QUIT",
							.update_callback = no_string,
							.action_callback = quit_button_action,
							.kb_shortcut = SDL_SCANCODE_Q
                        }
                }
        }
};

const m2::ui::UIBlueprint impl::ui::left_hud = {
        .w = 19, .h = 72,
        .background_color = {.r = 50, .g = 50, .b = 50, .a = 255},
        .elements = {
                m2::ui::ElementBlueprint{
                        .x = 4, .y = 50, .w = 11, .h = 2,
                        .variant = m2::ui::element::TextBlueprint{
							.initial_text = "HP",
							.update_callback = no_string,
							.action_callback = no_action
                        }
                },
                m2::ui::ElementBlueprint{
                        .x = 4, .y = 52, .w = 11, .h = 2,
                        .variant = m2::ui::element::TextBlueprint{
							.initial_text = "100",
							.update_callback = hp_update,
							.action_callback = no_action
                        }
                }
        }
};

const m2::ui::UIBlueprint impl::ui::right_hud = {
        .w = 19, .h = 72,
        .background_color = {.r = 50, .g = 50, .b = 50, .a = 255}
};
