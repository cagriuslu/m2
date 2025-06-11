#include <cuzn/ui/StatusBar.h>
#include <cuzn/ui/CustomHud.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2g_AttributeType.pb.h>

using namespace m2;
using namespace m2::widget;
using namespace m2g;

RectF status_bar_window_ratio() {
	const auto x = (1.0f - M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio()) / 2.0f;
	return {x, 0.0f, M2_GAME.Dimensions().GameWidthToGameAndHudWidthRatio(), 0.08f};
}

UiPanelBlueprint generate_status_bar_blueprint() {
	const auto turn_holder_index = M2_GAME.TurnHolderIndex();

	// Add player names
	auto bp = UiPanelBlueprint{
		.name = "StatusBar",
		.w = 70, .h = 8,
		.border_width = 0.0f,
		.onEvent = [=](const UiPanel& panel, const Events& events) -> UiAction {
			if (panel.Rect().DoesContain(events.MousePosition())) {
				if (not M2G_PROXY.custom_hud_panel) {
					M2G_PROXY.custom_hud_panel = M2_LEVEL.AddCustomNonblockingUiPanel(
							std::make_unique<UiPanelBlueprint>(generate_custom_hud_blueprint()),
							custom_hud_window_ratio());
				}
			} else {
				if (M2G_PROXY.custom_hud_panel) {
					M2_LEVEL.RemoveCustomNonblockingUiPanelDeferred(*M2G_PROXY.custom_hud_panel);
					M2G_PROXY.custom_hud_panel = std::nullopt;
				}
			}
			return MakeContinueAction();
		},
		.widgets = {
			// Gray background
			UiWidgetBlueprint{
				.x = 0, .y = 0, .w = 70, .h = 6,
				.border_width = 0.0f,
				.background_color = {80, 80, 80, 80},
				.variant = TextBlueprint{}
			},
			UiWidgetBlueprint{
				.x = 0, .y = 0, .w = 30, .h = 5,
				.border_width = 0.0f,
				.variant = TextBlueprint{
					.text = " Current Player Order",
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.wrapped_font_size_in_units = 4.0f
				}
			},
			UiWidgetBlueprint{
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

	int i = 0;
	for (auto order = m2g::pb::FIRST_PLAYER_INDEX;
			order <= m2g::pb::FORTH_PLAYER_INDEX;
			order = static_cast<m2g::pb::AttributeType>(I(order) + 1), ++i) {
		if (const auto playerIndexOfOrder = RoundI(M2G_PROXY.game_state_tracker().GetAttribute(order)); -1 < playerIndexOfOrder) {
				bp.widgets.emplace_back(UiWidgetBlueprint{
			// For deciding on the position, do not use the player index, use 0-based i
			.x = 30 + (4 - M2_GAME.TotalPlayerCount()) * 10 + i * 10, .y = 0, .w = 10, .h = 5,
			.border_width = 0.0f,
			.variant = TextBlueprint{
				.text = (playerIndexOfOrder == turn_holder_index)
						? "<" + generate_player_name(playerIndexOfOrder) + ">"
						:  generate_player_name(playerIndexOfOrder),
				.wrapped_font_size_in_units = 4.0f,
				.color = generate_player_color(playerIndexOfOrder),
			}
		});
		}
	}

	return bp;
}
