#include <cuzn/ui/CustomHud.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/sdl/Detail.h>

using namespace m2;
using namespace m2::widget;

RectF custom_hud_window_ratio() {
	const auto x = (1.0f - M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio()) / 2.0f;
	return {x, 0.0f, M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio(), 0.2f};
}

UiPanelBlueprint generate_custom_hud_blueprint(const int player_count) {
	auto turn_holder_index = M2_GAME.TurnHolderIndex();

	// Add player names
	auto bp = UiPanelBlueprint{
		.name = "CustomHud",
		.w = 70,
		.h = 20,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 0, .y = 0, .w = 30, .h = 5,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Current Player Order:",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 3.5f
				}
			}
		}
	};

	// Add victory points
	bp.widgets.emplace_back(UiWidgetBlueprint{
		.x = 0, .y = 5, .w = 30, .h = 5,
		.border_width = 0.0f,
		.variant = TextBlueprint{
			.text = " Victory/Income Points",
			.horizontal_alignment = TextHorizontalAlignment::LEFT,
			.wrapped_font_size_in_units = 3.5f
		}
	});

	// Add income points
	bp.widgets.emplace_back(UiWidgetBlueprint{
		.x = 0, .y = 10, .w = 30, .h = 5,
		.border_width = 0.0f,
		.variant = TextBlueprint{
			.text = " Money/Roads/Cards",
			.horizontal_alignment = TextHorizontalAlignment::LEFT,
			.wrapped_font_size_in_units = 3.5f
		}
	});

	// Add money spent this turn
	bp.widgets.emplace_back(UiWidgetBlueprint{
		.x = 0, .y = 15, .w = 30, .h = 5,
		.border_width = 0.0f,
		.variant = TextBlueprint{
			.text = " Money spent this turn",
			.horizontal_alignment = TextHorizontalAlignment::LEFT,
			.wrapped_font_size_in_units = 3.5f
		}
	});

	int i = 0;
	for (auto order = m2g::pb::FIRST_PLAYER_INDEX;
			order <= m2g::pb::FORTH_PLAYER_INDEX;
			order = static_cast<m2g::pb::AttributeType>(I(order) + 1), ++i) {
		if (const auto playerIndexOfOrder = iround(M2G_PROXY.game_state_tracker().get_attribute(order)); -1 < playerIndexOfOrder) {
			// Add player names
			bp.widgets.emplace_back(UiWidgetBlueprint{
				// For deciding on the position, do not use the player index, use 0-based i
				.x = 30 + (4 - M2_GAME.TotalPlayerCount()) * 10 + i * 10, .y = 0, .w = 10, .h = 5,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = (playerIndexOfOrder == turn_holder_index)
						? "<" + generate_player_name(playerIndexOfOrder) + ">"
						:  generate_player_name(playerIndexOfOrder),
					.wrapped_font_size_in_units = 3.5f,
					.color = generate_player_color(playerIndexOfOrder)
				}
			});

			// Add victory points
			auto& chr = M2_LEVEL.objects[M2G_PROXY.multiPlayerObjectIds[playerIndexOfOrder]].character();
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = 30 + (4 - M2_GAME.TotalPlayerCount()) * 10 + i * 10, .y = 5, .w = 10, .h = 5,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = m2::ToString(PlayerVictoryPoints(chr))
						+ "/" + m2::ToString(PlayerIncomePoints(chr)),
					.wrapped_font_size_in_units = 3.5f
				}
			});

			// Add income points
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = 30 + (4 - M2_GAME.TotalPlayerCount()) * 10 + i * 10, .y = 10, .w = 10, .h = 5,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = m2::ToString(PlayerMoney(chr))
						+ "/" + m2::ToString(PlayerUnbuiltRoadCount(chr))
						+ "/" + m2::ToString(PlayerCardCount(chr)),
					.wrapped_font_size_in_units = 3.5f
				}
			});

			// Add money spent this turn
			bp.widgets.emplace_back(UiWidgetBlueprint{
			.x = 30 + (4 - M2_GAME.TotalPlayerCount()) * 10 + i * 10, .y = 15, .w = 10, .h = 5,
			.border_width = 0.0f,
			.variant = TextBlueprint{
				.text = m2::ToString(M2G_PROXY.player_spent_money(playerIndexOfOrder)),
				.wrapped_font_size_in_units = 3.5f
			}
		});
		}
	}

	return bp;
}
