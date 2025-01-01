#include <cuzn/ui/StatusBar.h>
#include <cuzn/ui/CustomHud.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

RectF status_bar_window_ratio() {
	const auto x = (1.0f - M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio()) / 2.0f;
	return {x, 0.0f, M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio(), 0.08f};
}

PanelBlueprint generate_status_bar_blueprint(int player_count) {
	const auto turn_holder_index = M2_GAME.TurnHolderIndex();

	// Add player names
	auto bp = PanelBlueprint{
		.w = 70, .h = 8,
		.border_width = 0.0f,
		.on_event = [=](const Panel& panel, const Events& events) -> Action {
			if (panel.rect_px().contains(events.mouse_position())) {
				if (not M2G_PROXY.custom_hud_panel) {
					M2G_PROXY.custom_hud_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(
							std::make_unique<PanelBlueprint>(generate_custom_hud_blueprint(player_count)),
							custom_hud_window_ratio());
				}
			} else {
				if (M2G_PROXY.custom_hud_panel) {
					M2_LEVEL.remove_custom_nonblocking_ui_panel_deferred(*M2G_PROXY.custom_hud_panel);
					M2G_PROXY.custom_hud_panel = std::nullopt;
				}
			}
			return make_continue_action();
		},
		.widgets = {
			// Gray background
			WidgetBlueprint{
				.x = 0, .y = 0, .w = 70, .h = 6,
				.border_width = 0.0f,
				.background_color = {80, 80, 80, 80},
				.variant = TextBlueprint{}
			},
			WidgetBlueprint{
				.x = 0, .y = 0, .w = 30, .h = 5,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Current Player:",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 4.0f
				}
			},
			WidgetBlueprint{
				.x = 33, .y = 6, .w = 4, .h = 2,
				.border_width = 0.0f,
				.background_color = {80, 80, 80, 80},
				.variant = TextBlueprint{
					.text = "v",
					.wrapped_font_size_in_units = 2.0f
				}
			}
		}
	};
	for (int i = 0; i < player_count; ++i) {
		bp.widgets.emplace_back(WidgetBlueprint{
			.x = 70 - ((i + 1) * 10), .y = 0, .w = 10, .h = 5,
			.border_width = 0.0f,
			.variant = TextBlueprint{
				.text = (i == turn_holder_index)
						? "<" + generate_player_name(i) + ">"
						:  generate_player_name(i),
				.wrapped_font_size_in_units = 4.0f,
				.color = generate_player_color(i),
			}
		});
	}

	return bp;
}
