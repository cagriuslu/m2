#include <cuzn/ui/RightHud.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/ui/widget/TextSelection.h>
#include <cuzn/ui/Detail.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;
using namespace m2g;
using namespace m2g::pb;

static const Blueprint tiles_blueprint = {
	.w = 60,
	.h = 40,
	.border_width_px = 1,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 57,
			.y = 0,
			.w = 3,
			.h = 3,
			.variant =
			TextBlueprint{
				.text = "X",
				.on_action = [](MAYBE const m2::ui::widget::Text& self) -> m2::ui::Action {
					return make_return_action();
				}
			}
		},
		WidgetBlueprint{
			.name = "IndustryTypeSelection",
			.x = 3,
			.y = 3,
			.w = 18,
			.h = 34,
			.variant =
			TextSelectionBlueprint{
				.initial_list = {
					{m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_COTTON_MILL_TILE), static_cast<int>(m2g::pb::ITEM_CATEGORY_COTTON_MILL_TILE)},
					{m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_IRON_WORKS_TILE), static_cast<int>(m2g::pb::ITEM_CATEGORY_IRON_WORKS_TILE)},
					{m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_BREWERY_TILE), static_cast<int>(m2g::pb::ITEM_CATEGORY_BREWERY_TILE)},
					{m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_COAL_MINE_TILE), static_cast<int>(m2g::pb::ITEM_CATEGORY_COAL_MINE_TILE)},
					{m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_POTTERY_TILE), static_cast<int>(m2g::pb::ITEM_CATEGORY_POTTERY_TILE)},
					{m2g::pb::ItemCategory_Name(m2g::pb::ITEM_CATEGORY_MANUFACTURED_GOODS_TILE), static_cast<int>(m2g::pb::ITEM_CATEGORY_MANUFACTURED_GOODS_TILE)}},
				.line_count = 10,
				.allow_multiple_selection = false,
				.show_scroll_bar = false,
				.on_action = [](const TextSelection &self) -> Action {
					// Look for the other widget
					auto* tile_selection_widget = self.parent().find_first_widget_by_name<TextSelection>("TileSelection");
					if (tile_selection_widget) {
						// Trigger the other widget to recreate itself
						tile_selection_widget->reset();
					}
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.name = "TileSelection",
			.x = 21,
			.y = 3,
			.w = 18,
			.h = 34,
			.variant =
			TextSelectionBlueprint{
				.line_count = 10,
				.allow_multiple_selection = false,
				.show_scroll_bar = false,
				.on_create = [](TextSelection &self) {
					// Look for the other widget
					if (auto* industry_type_selection_widget = self.parent().find_first_widget_by_name<TextSelection>("IndustryTypeSelection")) {
						// Get selection
						auto selections = industry_type_selection_widget->selections();
						if (selections.size() == 1) {
							auto cat = static_cast<ItemCategory>(std::get<int>(selections[0]));
							TextSelectionBlueprint::Options options;
							for (auto item_it = M2_PLAYER.character().find_items(cat);
								 item_it != M2_PLAYER.character().end_items(); ++item_it) {
								options.emplace_back(m2g::pb::ItemType_Name(item_it->type()), m2::I(item_it->type()));
							}
							self.set_options(options);
						}
					}
				}
			}
		}
	}
};

const Blueprint right_hud_blueprint = {
	.w = 19,
	.h = 72,
	.border_width_px = 0,
	.background_color = {0, 0, 0, 255},
	.widgets = {
		WidgetBlueprint{
			.x = 2,
			.y = 1,
			.w = 15,
			.h = 6,
			.border_width_px = 0,
			.variant =
			TextBlueprint{
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_update = [](MAYBE Text& self) {
					auto vp = m2::I(M2_PLAYER.character().get_attribute(VICTORY_POINTS));
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
			.border_width_px = 0,
			.variant =
			TextBlueprint{
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_update = [](MAYBE Text& self) {
					auto vp = m2::I(M2_PLAYER.character().get_attribute(INCOME_POINTS));
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
			.border_width_px = 0,
			.variant =
			TextBlueprint{
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_update = [](MAYBE Text& self) {
					auto money = m2::I(M2_PLAYER.character().get_resource(MONEY));
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
			.border_width_px = 1,
			.variant =
			TextBlueprint{
				.text = "Cards",
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_action = [](MAYBE const Text& self) -> Action {
					M2_LEVEL.add_custom_ui_dialog(
						cards_window_ratio(),
						std::make_unique<Blueprint>(generate_cards_window("Cards")));
					return make_continue_action();
				}
			}
		},
		WidgetBlueprint{
			.x = 2,
			.y = 29,
			.w = 15,
			.h = 6,
			.border_width_px = 1,
			.variant = TextBlueprint{
				.text = "Tiles",
				.font_size = 4.5f,
				.alignment = m2::ui::TextAlignment::LEFT,
				.on_action = [](MAYBE const Text& self) -> Action {
					M2_LEVEL.add_custom_ui_dialog(m2::RectF{0.05f, 0.05f, 0.9f, 0.9f}, &tiles_blueprint);
					return make_continue_action();
				}
			}
		}
	}
};
