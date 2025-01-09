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
		.w = 7,
		.h = 4,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 0, .y = 0, .w = 3, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Current Player Order:",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 1, .w = 3, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Victory/Income Points",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 2, .w = 3, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Money/Roads/Cards",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 3, .w = 3, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Money spent this turn",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			}
		}
	};

	int i = 0;
	for (auto order = m2g::pb::FIRST_PLAYER_INDEX;
			order <= m2g::pb::FORTH_PLAYER_INDEX;
			order = static_cast<m2g::pb::AttributeType>(I(order) + 1), ++i) {
		if (const auto playerIndexOfOrder = iround(M2G_PROXY.game_state_tracker().get_attribute(order)); -1 < playerIndexOfOrder) {
			const auto x = 3 + (4 - M2_GAME.TotalPlayerCount()) + i;

			// Add player names
			bp.widgets.emplace_back(UiWidgetBlueprint{
				// For deciding on the position, do not use the player index, use 0-based i
				.x = x, .y = 0, .w = 1, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = (playerIndexOfOrder == turn_holder_index)
						? "<" + generate_player_name(playerIndexOfOrder) + ">"
						:  generate_player_name(playerIndexOfOrder),
					.wrapped_font_size_in_units = 0.7f,
					.color = generate_player_color(playerIndexOfOrder)
				}
			});

			// Add victory points
			auto& chr = M2_LEVEL.objects[M2G_PROXY.multiPlayerObjectIds[playerIndexOfOrder]].character();
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 1, .w = 1, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = m2::ToString(PlayerVictoryPoints(chr))
						+ "/" + m2::ToString(PlayerIncomePoints(chr)),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Add income points
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 2, .w = 1, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = m2::ToString(PlayerMoney(chr))
						+ "/" + m2::ToString(PlayerUnbuiltRoadCount(chr))
						+ "/" + m2::ToString(PlayerCardCount(chr)),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Add money spent this turn
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 3, .w = 1, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = m2::ToString(M2G_PROXY.player_spent_money(playerIndexOfOrder)),
					.wrapped_font_size_in_units = 0.7f
				}
			});
		}
	}

	return bp;
}
