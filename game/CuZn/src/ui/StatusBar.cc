#include <cuzn/ui/StatusBar.h>
#include <cuzn/ui/CustomHud.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

PanelBlueprint generate_status_bar_blueprint(int player_count) {
	auto turn_holder_index = M2_GAME.turn_holder_index();

	// Add player names
	auto bp = PanelBlueprint{
		.w = 70,
		.h = 9,
		.border_width = 0.0f,
		.background_color = {0, 0, 0, 45},
		.on_event = [=](Panel& panel, Events& events) -> Action {
			if (panel.rect_px().contains(events.mouse_position())) {
				// Create custom hud if not already
				if (not M2G_PROXY.custom_hud_panel) {
					M2G_PROXY.custom_hud_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(
						std::make_unique<m2::ui::PanelBlueprint>(generate_custom_hud_blueprint(player_count)),
						custom_hud_window_ratio());
				}
			}
			return make_continue_action();
		},
		.on_update = [](MAYBE Panel& panel) -> Action {
			// Clean up custom hud if already destroyed
			if (M2G_PROXY.custom_hud_panel && not (*M2G_PROXY.custom_hud_panel)->is_valid()) {
				M2_LEVEL.remove_custom_nonblocking_ui_panel(*M2G_PROXY.custom_hud_panel);
				M2G_PROXY.custom_hud_panel = std::nullopt;
			}
			return make_continue_action();
		},
		.widgets = {
			WidgetBlueprint{
				.x = 0, .y = 0, .w = 30, .h = 5,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = "Current Player",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 4.0f }
			},
			WidgetBlueprint{
				.x = 33, .y = 5, .w = 4, .h = 4,
				.border_width = 0.0f,
				.variant = TextBlueprint{ .text = "V" }
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
