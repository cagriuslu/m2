#include <cuzn/ui/CustomHud.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/sdl/Detail.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

PanelBlueprint generate_custom_hud_blueprint(int player_count) {
	auto turn_holder_index = M2_GAME.TurnHolderIndex();

	// Add player names
	auto bp = PanelBlueprint{
		.w = 70,
		.h = 20,
		.border_width = 0.0f,
		.background_color = {0, 0, 0, 255},
		.on_update = [](Panel& panel) -> Action {
			if (panel.rect_px().contains(sdl::mouse_position())) {
				panel.set_timeout(0.5f);
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
					.wrapped_font_size_in_units = 3.5f
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
				.wrapped_font_size_in_units = 3.5f,
				.color = generate_player_color(i)
			}
		});
	}

	// Add victory points
	bp.widgets.emplace_back(WidgetBlueprint{
		.x = 0, .y = 5, .w = 30, .h = 5,
		.border_width = 0.0f,
		.variant = TextBlueprint{
			.text = "Victory/Income Points",
			.horizontal_alignment = TextHorizontalAlignment::LEFT,
			.wrapped_font_size_in_units = 3.5f
		}
	});
	for (int i = 0; i < player_count; ++i) {
		auto& chr = M2_LEVEL.objects[M2G_PROXY.multi_player_object_ids[i]].character();
		bp.widgets.emplace_back(WidgetBlueprint{
			.x = 70 - ((i + 1) * 10), .y = 5, .w = 10, .h = 5,
			.border_width = 0.0f,
			.variant = TextBlueprint{
				.text = std::to_string(player_victory_points(chr))
					+ "/" + std::to_string(player_income_points(chr)),
				.wrapped_font_size_in_units = 3.5f
			}
		});
	}

	// Add income points
	bp.widgets.emplace_back(WidgetBlueprint{
		.x = 0, .y = 10, .w = 30, .h = 5,
		.border_width = 0.0f,
		.variant = TextBlueprint{
			.text = "Money/Roads/Cards",
			.horizontal_alignment = TextHorizontalAlignment::LEFT,
			.wrapped_font_size_in_units = 3.5f
		}
	});
	for (int i = 0; i < player_count; ++i) {
		auto& chr = M2_LEVEL.objects[M2G_PROXY.multi_player_object_ids[i]].character();
		bp.widgets.emplace_back(WidgetBlueprint{
			.x = 70 - ((i + 1) * 10), .y = 10, .w = 10, .h = 5,
			.border_width = 0.0f,
			.variant = TextBlueprint{
				.text = std::to_string(player_money(chr))
					+ "/" + std::to_string(player_available_road_count(chr))
					+ "/" + std::to_string(player_card_count(chr)),
				.wrapped_font_size_in_units = 3.5f
			}
		});
	}

	// Add money spent this turn
	bp.widgets.emplace_back(WidgetBlueprint{
		.x = 0, .y = 15, .w = 30, .h = 5,
		.border_width = 0.0f,
		.variant = TextBlueprint{
			.text = "Money spent this turn",
			.horizontal_alignment = TextHorizontalAlignment::LEFT,
			.wrapped_font_size_in_units = 3.5f
		}
	});
	for (int i = 0; i < player_count; ++i) {
		bp.widgets.emplace_back(WidgetBlueprint{
			.x = 70 - ((i + 1) * 10), .y = 15, .w = 10, .h = 5,
			.border_width = 0.0f,
			.variant = TextBlueprint{
				.text = std::to_string(M2G_PROXY.player_spent_money(i)),
				.wrapped_font_size_in_units = 3.5f
			}
		});
	}

	return bp;
}
