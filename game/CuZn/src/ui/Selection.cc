#include <cuzn/ui/Selection.h>
#include <m2/ui/UiPanelBlueprint.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;

bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text) {
	auto blueprint = UiPanelBlueprint{
		.name = "Confirmation",
		.w = 60, .h = 40,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 5, .y = 5, .w = 50, .h = 5,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = question1,
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			UiWidgetBlueprint{
				.x = 5, .y = 10, .w = 50, .h = 5,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = question2,
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			UiWidgetBlueprint{
				.x = 5, .y = 30, .w = 15, .h = 5,
				.variant = TextBlueprint{
					.text = decline_text,
					.on_action = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction<bool>(false);
					}
				}
			},
			UiWidgetBlueprint{
				.x = 25, .y = 30, .w = 30, .h = 5,
				.variant = TextBlueprint{
					.text = accept_text,
					.kb_shortcut = SDL_SCANCODE_RETURN,
					.on_action = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction<bool>(true);
					}
				}
			}
		}
	};

	bool selection;
	auto background = M2_GAME.DrawGameToTexture(M2_LEVEL.camera()->position);
	UiPanel::create_and_run_blocking(&blueprint, RectF{0.15f, 0.15f, 0.7f, 0.7f}, std::move(background))
			.IfQuit([] { M2_GAME.quit = true; })
			.IfReturn<bool>([&](auto result) { selection = result; });
	return selection;
}

std::optional<bool> ask_for_confirmation_with_cancellation(const std::string& question, const std::string& accept_text, const std::string& decline_text) {
	auto blueprint = UiPanelBlueprint{
		.name = "ConfirmationWithCancellation",
		.w = 80, .h = 45,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 5, .y = 5, .w = 70, .h = 25,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = question,
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.vertical_alignment = m2::TextVerticalAlignment::TOP,
					.wrapped_font_size_in_units = 3.0f
				}
			},
			UiWidgetBlueprint{
				.x = 5, .y = 35, .w = 20, .h = 5,
				.variant = TextBlueprint{
					.text = accept_text,
					.wrapped_font_size_in_units = 3.0f,
					.kb_shortcut = SDL_SCANCODE_RETURN,
					.on_action = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction<bool>(true);
					}
				}
			},
			UiWidgetBlueprint{
				.x = 30, .y = 35, .w = 20, .h = 5,
				.variant = TextBlueprint{
					.text = decline_text,
					.wrapped_font_size_in_units = 3.0f,
					.on_action = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction<bool>(false);
					}
				}
			},
			UiWidgetBlueprint{
				.x = 55, .y = 35, .w = 20, .h = 5,
				.variant = TextBlueprint{
					.text = "Cancel",
					.wrapped_font_size_in_units = 3.0f,
					.on_action = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction();
					}
				}
			}
		}
	};

	std::optional<bool> selection;
	auto background = M2_GAME.DrawGameToTexture(M2_LEVEL.camera()->position);
	UiPanel::create_and_run_blocking(&blueprint, RectF{0.25f, 0.25f, 0.5f, 0.5f}, std::move(background))
			.IfQuit([] { M2_GAME.quit = true; })
			.IfReturn<bool>([&](auto result) { selection = result; });
	return selection;
}

bool ask_for_confirmation_bottom(const std::string& question, const std::string& accept_text, const std::string& decline_text, m2::sdl::TextureUniquePtr background_texture) {
	auto blueprint = UiPanelBlueprint{
		.name = "ConfirmationOnBottom",
		.w = 44, .h = 12,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1, .y = 1, .w = 32, .h = 10,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = question,
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			UiWidgetBlueprint{
				.x = 34, .y = 1, .w = 4, .h = 10,
				.variant = TextBlueprint{
					.text = decline_text,
					.on_action = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction<bool>(false);
					}
				}
			},
			UiWidgetBlueprint{
				.x = 39, .y = 1, .w = 4, .h = 10,
				.variant = TextBlueprint{
					.text = accept_text,
					.kb_shortcut = SDL_SCANCODE_RETURN,
					.on_action = [](MAYBE const Text& self) -> UiAction {
						return MakeReturnAction<bool>(true);
					}
				}
			}
		}
	}; // 1 + 32 + 1 + 4 + 1 + 4 + 1

	bool selection;
	UiPanel::create_and_run_blocking(&blueprint, M2_GAME.Dimensions().Game().ratio({0.0f, 0.9f, 1.0f, 0.1f}), std::move(background_texture))
			.IfQuit([] { M2_GAME.quit = true; })
			.IfReturn<bool>([&](auto result) { selection = result; });
	return selection;
}
