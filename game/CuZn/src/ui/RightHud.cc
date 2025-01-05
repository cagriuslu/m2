#include <cuzn/ui/RightHud.h>
#include <cuzn/detail/Income.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextSelection.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <cuzn/ui/Market.h>
#include <cuzn/ui/Tiles.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::widget;
using namespace m2g;
using namespace m2g::pb;

const UiPanelBlueprint right_hud_blueprint = {
	.name = "RightHud",
	.w = 19,
	.h = 72,
	.border_width = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		UiWidgetBlueprint{
			.x = 2,
			.y = 2,
			.w = 15,
			.h = 4,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.horizontal_alignment = TextHorizontalAlignment::LEFT,
				.wrapped_font_size_in_units = 1.25f,
				.on_update = [](MAYBE Text& self) {
					auto vp = m2::iround(M2_PLAYER.character().get_resource(VICTORY_POINTS));
					self.set_text(std::string{"Victory Points: "} + m2::ToString(vp));
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 2,
			.y = 7,
			.w = 15,
			.h = 4,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.horizontal_alignment = TextHorizontalAlignment::LEFT,
				.wrapped_font_size_in_units = 1.25f,
				.on_update = [](MAYBE Text& self) {
					auto vp = m2::iround(M2_PLAYER.character().get_attribute(INCOME_POINTS));
					self.set_text(std::string{"Income Points: "} + m2::ToString(vp));
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 2,
			.y = 12,
			.w = 15,
			.h = 4,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.horizontal_alignment = TextHorizontalAlignment::LEFT,
				.wrapped_font_size_in_units = 1.25f,
				.on_update = [](MAYBE Text& self) {
					const auto income_points = m2::iround(M2_PLAYER.character().get_attribute(INCOME_POINTS));
					const auto income_level = IncomeLevelFromIncomePoints(income_points);
					self.set_text(std::string{"Income: £"} + m2::ToString(income_level));
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 2,
			.y = 17,
			.w = 15,
			.h = 4,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.horizontal_alignment = TextHorizontalAlignment::LEFT,
				.wrapped_font_size_in_units = 1.25f,
				.on_update = [](MAYBE Text& self) {
					auto money = m2::iround(M2_PLAYER.character().get_resource(MONEY));
					self.set_text(std::string{"Cash: £"} + m2::ToString(money));
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 2,
			.y = 22,
			.w = 15,
			.h = 4,
			.variant =
			TextBlueprint{
				.text = "Cards",
				.horizontal_alignment = m2::TextHorizontalAlignment::CENTER,
				.wrapped_font_size_in_units = 1.75f,
				.on_action = [](MAYBE const Text& self) -> UiAction {
					// Check if the panel is still active
					if (M2G_PROXY.cards_panel) {
						if ((*M2G_PROXY.cards_panel)->IsKilled()) {
							// Cards panel have been killed
							M2_LEVEL.remove_custom_nonblocking_ui_panel(*M2G_PROXY.cards_panel);
							M2G_PROXY.cards_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(
									std::make_unique<UiPanelBlueprint>(generate_cards_window("Cards")),
									cards_panel_ratio());
						} else {
							M2_LEVEL.remove_custom_nonblocking_ui_panel(*M2G_PROXY.cards_panel);
							M2G_PROXY.cards_panel.reset();
						}
					} else {
						M2G_PROXY.cards_panel = M2_LEVEL.add_custom_nonblocking_ui_panel(
								std::make_unique<UiPanelBlueprint>(generate_cards_window("Cards")),
								cards_panel_ratio());
					}
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 2,
			.y = 27,
			.w = 15,
			.h = 4,
			.variant = TextBlueprint{
				.text = "Tiles",
				.horizontal_alignment = m2::TextHorizontalAlignment::CENTER,
				.wrapped_font_size_in_units = 1.75f,
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ShowSemiBlockingUiPanel(tiles_window_ratio(), std::make_unique<UiPanelBlueprint>(generate_tiles_window("Tiles")));
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 2,
			.y = 32,
			.w = 15,
			.h = 4,
			.variant = TextBlueprint{
				.text = "Market",
				.horizontal_alignment = m2::TextHorizontalAlignment::CENTER,
				.wrapped_font_size_in_units = 1.75f,
				.on_action = [](MAYBE const Text& self) -> UiAction {
					M2_LEVEL.ShowSemiBlockingUiPanel(market_window_ratio(), std::make_unique<UiPanelBlueprint>(generate_market_window()));
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 2,
			.y = 37,
			.w = 15,
			.h = 8,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.horizontal_alignment = m2::TextHorizontalAlignment::CENTER,
				.wrapped_font_size_in_units = 1.35f,
				.on_update = [](MAYBE Text& self) {
					auto dds = m2::iround(M2G_PROXY.game_state_tracker().get_resource(DRAW_DECK_SIZE));
					self.set_text(std::string{"Cards Left in Deck: "} + m2::ToString(dds));
					return MakeContinueAction();
				}
			}
		}
	}
};
