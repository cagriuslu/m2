#include <cuzn/ui/CustomHud.h>
#include <cuzn/detail/Income.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>

using namespace m2;
using namespace m2::widget;

RectF custom_hud_window_ratio() {
	const auto x = (1.0f - M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio()) / 2.0f;
	return {x, 0.0f, M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio(), 0.45f};
}

UiPanelBlueprint generate_custom_hud_blueprint() {
	auto turn_holder_index = M2_GAME.TurnHolderIndex();

	// Add player names
	auto bp = UiPanelBlueprint{
		.name = "CustomHud",
		.w = 7,
		.h = 9,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 0, .y = 0, .w = 3, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Current Player Order",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 1, .w = 3, .h = 1,
				.border_width = 0.0f,
				.background_color = {42, 42, 42, 255},
				.variant = TextBlueprint{
					.text = " Victory Points",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 2, .w = 3, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Estimated Victory Points",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 3, .w = 3, .h = 1,
				.border_width = 0.0f,
				.background_color = {42, 42, 42, 255},
				.variant = TextBlueprint{
					.text = " Income Points",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 4, .w = 3, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Income",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 5, .w = 3, .h = 1,
				.border_width = 0.0f,
				.background_color = {42, 42, 42, 255},
				.variant = TextBlueprint{
					.text = " Money",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 6, .w = 3, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Roads",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 7, .w = 3, .h = 1,
				.border_width = 0.0f,
				.background_color = {42, 42, 42, 255},
				.variant = TextBlueprint{
					.text = " Cards",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 0.7f
				}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 8, .w = 3, .h = 1,
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
		if (const auto playerIndexOfOrder = RoundI(M2G_PROXY.game_state_tracker().GetAttribute(order)); -1 < playerIndexOfOrder) {
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

			auto& chr = M2_LEVEL.objects[M2G_PROXY.multiPlayerObjectIds[playerIndexOfOrder]].GetCharacter();
			// Victory points
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 1, .w = 1, .h = 1,
				.border_width = 0.0f,
				.background_color = {42, 42, 42, 255},
				.variant = TextBlueprint{
					.text = ToString(PlayerVictoryPoints(chr)),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Estimated victory points
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 2, .w = 1, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = ToString(PlayerLinkCount(chr) + PlayerEstimatedVictoryPoints(chr)),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Income points
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 3, .w = 1, .h = 1,
				.border_width = 0.0f,
				.background_color = {42, 42, 42, 255},
				.variant = TextBlueprint{
					.text = ToString(PlayerIncomePoints(chr)),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Income
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 4, .w = 1, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = ToString(IncomeLevelFromIncomePoints(PlayerIncomePoints(chr))),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Money
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 5, .w = 1, .h = 1,
				.border_width = 0.0f,
				.background_color = {42, 42, 42, 255},
				.variant = TextBlueprint{
					.text = ToString(PlayerMoney(chr)),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Roads
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 6, .w = 1, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = ToString(PlayerUnbuiltRoadCount(chr)),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Cards
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 7, .w = 1, .h = 1,
				.border_width = 0.0f,
				.background_color = {42, 42, 42, 255},
				.variant = TextBlueprint{
					.text = ToString(PlayerCardCount(chr)),
					.wrapped_font_size_in_units = 0.7f
				}
			});

			// Money spent this turn
			bp.widgets.emplace_back(UiWidgetBlueprint{
				.x = x, .y = 8, .w = 1, .h = 1,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = ToString(M2G_PROXY.player_spent_money(playerIndexOfOrder)),
					.wrapped_font_size_in_units = 0.7f
				}
			});
		}
	}

	return bp;
}
