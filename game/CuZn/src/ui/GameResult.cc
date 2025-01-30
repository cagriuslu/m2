#include <cuzn/ui/GameResult.h>
#include <m2/ui/UiPanel.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <m2/ui/widget/Text.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2;

void display_game_result() {
	LOG_INFO("Displaying GameResult");

	// Lookup the status from the game status
	auto blueprint = UiPanelBlueprint{
		.name = "GameResult",
		.w = 12, .h = 13,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1, .y = 1, .w = 10, .h = 1,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = "Game has ended",
					.wrapped_font_size_in_units = 0.8f
				}
			},
			UiWidgetBlueprint{
				.x = 1, .y = 11, .w = 10, .h = 1,
				.variant = widget::TextBlueprint{
					.text = "OK",
					.wrapped_font_size_in_units = 0.8f,
					.kb_shortcut = SDL_SCANCODE_RETURN,
					.onAction = [](MAYBE const widget::Text& self) -> UiAction { return MakeReturnAction(); }
				}
			}
		}
	};
	for (int i = 0; i < I(M2G_PROXY.multiPlayerObjectIds.size()); ++i) {
		auto id = M2G_PROXY.multiPlayerObjectIds[i];
		auto& player_obj = M2_LEVEL.objects[id];
		auto& player_chr = player_obj.character();
		auto victory_points = iround(player_chr.get_resource(m2g::pb::VICTORY_POINTS));

		blueprint.widgets.emplace_back(
			UiWidgetBlueprint{
				.x = 1, .y = 3 + (2 * i), .w = 6, .h = 1,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = generate_player_name(i),
					.horizontal_alignment = m2::TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.8f,
					.color = generate_player_color(i)
				}
			});
		blueprint.widgets.emplace_back(
			UiWidgetBlueprint{
				.x = 8, .y = 3 + (2 * i), .w = 3, .h = 1,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = m2::ToString(victory_points),
					.horizontal_alignment = m2::TextHorizontalAlignment::RIGHT,
					.wrapped_font_size_in_units = 0.8f
				}
			});
	}

	auto background = M2_GAME.DrawGameToTexture(M2_LEVEL.Camera()->position);
	UiPanel::create_and_run_blocking(&blueprint, RectF{0.15f, 0.15f, 0.7f, 0.7f}, std::move(background))
			.IfQuit([] { M2_GAME.quit = true; });
}
