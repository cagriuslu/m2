#include <cuzn/ui/Tiles.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/Widget.h>
#include <m2/ui/State.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::ui;
using namespace m2::ui::widget;

m2::RectF tiles_window_ratio() {
	return m2::RectF{0.05f, 0.05f, 0.9f, 0.9f};
}

m2::ui::Blueprint generate_tiles_window(const std::string& msg, m2g::pb::ItemType exclude_tile) {
	return Blueprint{
		.w = 61,
		.h = 26,
		.border_width_px = 1,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 1,
				.y = 1,
				.w = 59,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{ .text = msg }
			},
			WidgetBlueprint{
				.name = "IndustryTypeSelection",
				.x = 1,
				.y = 4,
				.w = 18,
				.h = 16,
				.variant =
				TextSelectionBlueprint{
					.initial_list = {
						{"Cotton Mill", static_cast<int>(m2g::pb::ITEM_CATEGORY_COTTON_MILL_TILE)},
						{"Iron Works", static_cast<int>(m2g::pb::ITEM_CATEGORY_IRON_WORKS_TILE)},
						{"Brewery", static_cast<int>(m2g::pb::ITEM_CATEGORY_BREWERY_TILE)},
						{"Coal Mine", static_cast<int>(m2g::pb::ITEM_CATEGORY_COAL_MINE_TILE)},
						{"Pottery", static_cast<int>(m2g::pb::ITEM_CATEGORY_POTTERY_TILE)},
						{"Manufactured Goods", static_cast<int>(m2g::pb::ITEM_CATEGORY_MANUFACTURED_GOODS_TILE)}},
					.line_count = 11,
					.allow_multiple_selection = false,
					.show_scroll_bar = false,
					.on_action = [](const TextSelection &self) -> Action {
						// Look for the other widget
						auto* tile_selection_widget = self.parent().find_first_widget_by_name<TextSelection>("TileSelection");
						tile_selection_widget->reset(); // Trigger the other widget to recreate itself
						// Clear details
						self.parent().find_first_widget_by_name<Text>("BuildRequirements")->set_text("");
						self.parent().find_first_widget_by_name<Text>("ResourceGain")->set_text("");
						self.parent().find_first_widget_by_name<Text>("SellRequirements")->set_text("");
						self.parent().find_first_widget_by_name<Text>("SellBenefits")->set_text("");
						return make_continue_action();
					}
				}
			},
			WidgetBlueprint{
				.name = "TileSelection",
				.x = 20,
				.y = 4,
				.w = 10,
				.h = 16,
				.variant =
				TextSelectionBlueprint{
					.line_count = 11,
					.allow_multiple_selection = false,
					.show_scroll_bar = false,
					.on_create = [exclude_tile](TextSelection &self) {
						// Look for the other widget
						auto* industry_type_selection_widget = self.parent().find_first_widget_by_name<TextSelection>("IndustryTypeSelection");
						if (auto selections = industry_type_selection_widget->selections(); not selections.empty()) {
							auto cat = static_cast<m2g::pb::ItemCategory>(std::get<int>(selections[0]));
							TextSelectionBlueprint::Options options;
							for (auto item_it = M2_PLAYER.character().find_items(cat); item_it != M2_PLAYER.character().end_items(); ++item_it) {
								if (item_it->type() != exclude_tile) {
									options.emplace_back(M2_GAME.get_named_item(item_it->type()).in_game_name(), m2::I(item_it->type()));
								}
							}
							self.set_options(options);
						}
					},
					.on_action = [](TextSelection &self) -> Action {
						if (auto selections = self.selections(); not selections.empty()) {
							auto selected_item_type = static_cast<m2g::pb::ItemType>(std::get<int>(selections[0]));
							const auto& selected_item = M2_GAME.get_named_item(selected_item_type);
							{
								std::string build_requirements = "£" + std::to_string(iround(selected_item.get_attribute(m2g::pb::MONEY_COST)));
								if (auto coal_cost = iround(selected_item.get_attribute(m2g::pb::COAL_COST))) {
									build_requirements += ", " + std::to_string(coal_cost) + " Coal";
								}
								if (auto iron_cost = iround(selected_item.get_attribute(m2g::pb::IRON_COST))) {
									build_requirements += ", " + std::to_string(iron_cost) + " Iron";
								}
								self.parent().find_first_widget_by_name<Text>("BuildRequirements")->set_text(build_requirements);
							}
							{
								std::string resource_gain;
								if (auto coal_gain = iround(selected_item.get_attribute(m2g::pb::COAL_BONUS))) {
									resource_gain = std::to_string(coal_gain) + " Coal";
								} else if (auto iron_gain = iround(selected_item.get_attribute(m2g::pb::IRON_BONUS))) {
									resource_gain = std::to_string(iron_gain) + " Iron";
								} else if (auto canal_era_beer_gain = iround(selected_item.get_attribute(m2g::pb::BEER_BONUS_FIRST_ERA))) {
									resource_gain = std::to_string(canal_era_beer_gain);
									auto railroad_era_beer_gain = iround(selected_item.get_attribute(m2g::pb::BEER_BONUS_SECOND_ERA));
									if (railroad_era_beer_gain != canal_era_beer_gain) {
										resource_gain += "/" + std::to_string(railroad_era_beer_gain);
									}
									resource_gain += " Beer";
								}
								self.parent().find_first_widget_by_name<Text>("ResourceGain")->set_text(resource_gain);
							}
							{
								if (auto sell_beer_cost = iround(selected_item.get_attribute(m2g::pb::BEER_COST))) {
									std::string sell_requirements = std::to_string(sell_beer_cost) + " Beer";
									self.parent().find_first_widget_by_name<Text>("SellRequirements")->set_text(sell_requirements);
								} else {
									self.parent().find_first_widget_by_name<Text>("SellRequirements")->set_text("");
								}
							}
							{
								std::string sell_benefits = std::to_string(iround(selected_item.get_attribute(m2g::pb::VICTORY_POINTS_BONUS))) + " Points, ";
								sell_benefits += std::to_string(iround(selected_item.get_attribute(m2g::pb::INCOME_POINTS_BONUS))) + " Income, ";
								sell_benefits += std::to_string(iround(selected_item.get_attribute(m2g::pb::LINK_BONUS))) + " Link";
								self.parent().find_first_widget_by_name<Text>("SellBenefits")->set_text(sell_benefits);
							}
						} else {
							// Clear details
							self.parent().find_first_widget_by_name<Text>("BuildRequirements")->set_text("");
							self.parent().find_first_widget_by_name<Text>("ResourceGain")->set_text("");
							self.parent().find_first_widget_by_name<Text>("SellRequirements")->set_text("");
							self.parent().find_first_widget_by_name<Text>("SellBenefits")->set_text("");
						}
						return make_continue_action();
					}
				}
			},
			WidgetBlueprint{
				.x = 31,
				.y = 4,
				.w = 29,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.text = "Build requirements:",
					.font_size = 1.5f,
					.alignment = TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.name = "BuildRequirements",
				.x = 31,
				.y = 6,
				.w = 29,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.font_size = 1.9f,
					.alignment = TextAlignment::RIGHT
				}
			},
			WidgetBlueprint{
				.x = 31,
				.y = 8,
				.w = 29,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.text = "Resource gain:",
					.font_size = 1.5f,
					.alignment = TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.name = "ResourceGain",
				.x = 31,
				.y = 10,
				.w = 29,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.font_size = 1.9f,
					.alignment = TextAlignment::RIGHT
				}
			},
			WidgetBlueprint{
				.x = 31,
				.y = 12,
				.w = 29,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.text = "Sell requirements:",
					.font_size = 1.5f,
					.alignment = TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.name = "SellRequirements",
				.x = 31,
				.y = 14,
				.w = 29,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.font_size = 1.9f,
					.alignment = TextAlignment::RIGHT
				}
			},
			WidgetBlueprint{
				.x = 31,
				.y = 16,
				.w = 29,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.text = "Sell benefits:",
					.font_size = 1.5f,
					.alignment = TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.name = "SellBenefits",
				.x = 31,
				.y = 18,
				.w = 29,
				.h = 2,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.font_size = 1.9f,
					.alignment = TextAlignment::RIGHT
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 21,
				.w = 59,
				.h = 1,
				.border_width_px = 0,
				.variant = TextBlueprint{
					.text = "CO = Canal Only   RRO = Railroad Only   ND = No Develop",
					.alignment = TextAlignment::LEFT
				}
			},
			WidgetBlueprint{
				.x = 1,
				.y = 23,
				.w = 59,
				.h = 2,
				.variant = TextBlueprint{
					.text = "OK",
					.on_action = [](const Text& self) -> Action {
						// Find the other blueprint
						if (auto* tile_selection = self.parent().find_first_widget_by_name<TextSelection>("TileSelection")) {
							if (auto selections = tile_selection->selections(); not selections.empty()) {
								auto item_type = static_cast<m2g::pb::ItemType>(std::get<int>(selections[0]));
								return make_return_action<m2g::pb::ItemType>(item_type);
							}
						}
						return make_return_action();
					}
				}
			}
		}
	};
}

std::optional<m2g::pb::ItemType> ask_for_tile_selection(m2g::pb::ItemType exclude_tile) {
	LOG_INFO("Asking player to select a tile...");
	std::optional<m2g::pb::ItemType> selected_tile;
	State::create_execute_sync(std::make_unique<Blueprint>(generate_tiles_window("Select tile to develop", exclude_tile)), M2_GAME.dimensions().game_and_hud.ratio(tiles_window_ratio()))
		.if_void_return([&]() {
			LOG_INFO("Tile selection cancelled");
		})
		.if_return<m2g::pb::ItemType>([&selected_tile](auto picked_tile) {
			LOG_INFO("Tile selected", m2g::pb::ItemType_Name(picked_tile));
			selected_tile = picked_tile;
		});
	return selected_tile;
}