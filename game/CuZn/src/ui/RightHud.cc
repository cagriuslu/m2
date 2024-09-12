#include <cuzn/ui/RightHud.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextSelection.h>
#include <cuzn/ui/Detail.h>
#include <cuzn/ui/Cards.h>
#include <cuzn/ui/Market.h>
#include <cuzn/ui/Tiles.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

const PanelBlueprint right_hud_blueprint = {
	.w = 19,
	.h = 72,
	.border_width = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 2,
			.y = 1,
			.w = 15,
			.h = 6,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::LEFT,
				.on_update = [](MAYBE Text& self) {
					auto vp = m2::iround(M2_PLAYER.character().get_resource(VICTORY_POINTS));
					self.set_text(std::string{"Points:"} + std::to_string(vp));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 8,
			.w = 15,
			.h = 6,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::LEFT,
				.on_update = [](MAYBE Text& self) {
					auto vp = m2::iround(M2_PLAYER.character().get_attribute(INCOME_POINTS));
					self.set_text(std::string{"Income:"} + std::to_string(vp));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 15,
			.w = 15,
			.h = 6,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::LEFT,
				.on_update = [](MAYBE Text& self) {
					auto money = m2::iround(M2_PLAYER.character().get_resource(MONEY));
					self.set_text(std::string{"Cash:£"} + std::to_string(money));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 22,
			.w = 15,
			.h = 6,
			.variant =
			TextBlueprint{
				.text = "Cards",
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::CENTER,
				.on_action = [](MAYBE const Text& self) -> Action {
					M2_LEVEL.add_custom_blocking_ui_panel(
						cards_window_ratio(),
						std::make_unique<PanelBlueprint>(generate_cards_window("Cards")));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 29,
			.w = 15,
			.h = 6,
			.variant = TextBlueprint{
				.text = "Tiles",
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::CENTER,
				.on_action = [](MAYBE const Text& self) -> Action {
					M2_LEVEL.add_custom_blocking_ui_panel(tiles_window_ratio(), std::make_unique<PanelBlueprint>(generate_tiles_window("Tiles")));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 36,
			.w = 15,
			.h = 6,
			.border_width = 0,
			.variant =
			TextBlueprint{
				.on_update = [](MAYBE Text& self) {
					auto dds = m2::iround(M2G_PROXY.game_state_tracker().get_resource(DRAW_DECK_SIZE));
					self.set_text(std::string{"Deck:"} + std::to_string(dds));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 43,
			.w = 15,
			.h = 6,
			.variant = TextBlueprint{
				.text = "Market",
				.horizontal_alignment = m2::ui::TextHorizontalAlignment::CENTER,
				.on_action = [](MAYBE const Text& self) -> Action {
					M2_LEVEL.add_custom_blocking_ui_panel(market_window_ratio(), std::make_unique<PanelBlueprint>(generate_market_window()));
					return make_continue_action();
				}
			}
		}
	}
};
