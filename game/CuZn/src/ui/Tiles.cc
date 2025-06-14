#include <cuzn/ui/Tiles.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/UiWidget.h>
#include <m2/ui/UiPanel.h>
#include <m2/Log.h>
#include <m2/protobuf/Detail.h>
#include <m2/ui/widget/Image.h>

using namespace m2;
using namespace m2::widget;

namespace {
	struct TileComparator {
		static int roman_to_int(const std::string& r) {
			if (r == "I") { return 1; }
			if (r == "II") { return 2; }
			if (r == "III") { return 3; }
			if (r == "IV") { return 4; }
			if (r == "V") { return 5; }
			if (r == "VI") { return 6; }
			if (r == "VII") { return 7; }
			if (r == "VIII") { return 8; }
			throw M2_ERROR("Unexpected roman numeral");
		}

		// The tile names end with _TILE_YYY where YYY is a Roman numeral
		bool operator()(const IndustryTile a, const IndustryTile b) const {
			const auto name_a = pb::enum_name(a);
			const auto name_b = pb::enum_name(b);
			const std::string search_for = "_TILE_";
			const auto pos_a = name_a.rfind(search_for);
			const auto pos_b = name_b.rfind(search_for);
			const auto roman_a = name_a.substr(pos_a + search_for.size());
			const auto roman_b = name_b.substr(pos_b + search_for.size());
			return roman_to_int(roman_a) < roman_to_int(roman_b);
		}
	};
}

RectF tiles_window_ratio() {
	return RectF{0.05f, 0.05f, 0.9f, 0.9f};
}

UiPanelBlueprint generate_tiles_window(const std::string& msg, m2g::pb::ItemType exclude_tile) {
	return UiPanelBlueprint{
		.name = "TilesWindow",
		.w = 61,
		.h = 26,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			UiWidgetBlueprint{
				.x = 1,
				.y = 1,
				.w = 59,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{ .text = msg }
			},
			UiWidgetBlueprint{
				.name = "IndustryTypeSelection",
				.x = 1,
				.y = 4,
				.w = 18,
				.h = 10,
				.variant = TextSelectionBlueprint{
					.options = {
						{"Cotton Mill", static_cast<int>(m2g::pb::ITEM_CATEGORY_COTTON_MILL_TILE)},
						{"Iron Works", static_cast<int>(m2g::pb::ITEM_CATEGORY_IRON_WORKS_TILE)},
						{"Brewery", static_cast<int>(m2g::pb::ITEM_CATEGORY_BREWERY_TILE)},
						{"Coal Mine", static_cast<int>(m2g::pb::ITEM_CATEGORY_COAL_MINE_TILE)},
						{"Pottery", static_cast<int>(m2g::pb::ITEM_CATEGORY_POTTERY_TILE)},
						{"Manufactured Goods", static_cast<int>(m2g::pb::ITEM_CATEGORY_MANUFACTURED_GOODS_TILE)}},
					.line_count = 6,
					.allow_multiple_selection = false,
					.show_scroll_bar = false,
					.onAction = [exclude_tile](const TextSelection &self) -> UiAction {
						auto tile_to_filter = exclude_tile; // Create a copy because it'll be mutated once the filtered tile is encountered
						if (auto industry_type_selections = self.GetSelectedOptions(); not industry_type_selections.empty()) {
							auto industry_type_selection = industry_type_selections[0];
							auto industry_type_selection_int = I(industry_type_selection);
							auto industry_type = static_cast<m2g::pb::ItemCategory>(industry_type_selection_int);
							// Gather the industry tiles
							std::vector<IndustryTile> industry_tiles;
							for (auto item_it = M2_PLAYER.GetCharacter().FindItems(industry_type); item_it != M2_PLAYER.GetCharacter().EndItems(); ++item_it) {
								if (item_it->Type() == tile_to_filter) {
									// Don't emplace, clear filter because only one tile is filtered
									tile_to_filter = {};
								} else {
									industry_tiles.emplace_back(item_it->Type());
								}
							}
							// Sort the tiles
							std::ranges::sort(industry_tiles, TileComparator{});
							// Look up tile names
							TextSelectionBlueprint::Options options;
							std::ranges::transform(industry_tiles, std::back_inserter(options), [](const IndustryTile tile) {
								return TextSelectionBlueprint::Option{M2_GAME.GetNamedItem(tile).in_game_name(), I(tile)};
							});
							// Look for the other widget
							self.Parent().FindWidget<TextSelection>("TileLevelSelection")
									->SetOptions(std::move(options)); // Trigger the other widget to recreate itself
							const auto industry = industry_of_industry_tile_category(industry_type);
							self.Parent().FindWidget<Image>("IndustryVisual")
									->SetSpriteType(M2_GAME.GetNamedItem(industry).UiSprite()); // Set the industry image
						} else {
							self.Parent().FindWidget<TextSelection>("TileLevelSelection")
									->SetOptions({}); // Trigger the other widget to recreate itself
							self.Parent().FindWidget<Image>("IndustryVisual")
									->SetSpriteType({}); // Clear the industry image
						}
						// Clear details
						self.Parent().FindWidget<Text>("BuildRequirements")->set_text("");
						self.Parent().FindWidget<Text>("ResourceGain")->set_text("");
						self.Parent().FindWidget<Text>("SellRequirements")->set_text("");
						self.Parent().FindWidget<Text>("SellBenefits")->set_text("");
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.name = "IndustryVisual",
				.x = 6,
				.y = 15,
				.w = 8,
				.h = 5,
				.variant = ImageBlueprint{}
			},
			UiWidgetBlueprint{
				.name = "TileLevelSelection",
				.x = 20,
				.y = 4,
				.w = 10,
				.h = 16,
				.variant = TextSelectionBlueprint{
					.line_count = 11,
					.allow_multiple_selection = false,
					.show_scroll_bar = false,
					.onAction = [](const TextSelection &self) -> UiAction {
						if (auto selections = self.GetSelectedOptions(); not selections.empty()) {
							const auto selectedTileType = static_cast<m2g::pb::ItemType>(I(selections[0]));
							self.Parent().FindWidget<Text>("BuildRequirements")->set_text(GetIndustryTileBuildRequirementsString(selectedTileType));
							self.Parent().FindWidget<Text>("ResourceGain")->set_text(GetIndustryTileResourceGainString(selectedTileType));
							self.Parent().FindWidget<Text>("SellRequirements")->set_text(GetIndustryTileSellRequirementsString(selectedTileType));
							self.Parent().FindWidget<Text>("SellBenefits")->set_text(GetIndustryTileSellBenefitsString(selectedTileType));
						} else {
							// Clear details
							self.Parent().FindWidget<Text>("BuildRequirements")->set_text("");
							self.Parent().FindWidget<Text>("ResourceGain")->set_text("");
							self.Parent().FindWidget<Text>("SellRequirements")->set_text("");
							self.Parent().FindWidget<Text>("SellBenefits")->set_text("");
						}
						return MakeContinueAction();
					}
				}
			},
			UiWidgetBlueprint{
				.x = 31,
				.y = 4,
				.w = 29,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Build requirements:",
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			UiWidgetBlueprint{
				.name = "BuildRequirements",
				.x = 31,
				.y = 6,
				.w = 29,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{
					.horizontal_alignment = TextHorizontalAlignment::RIGHT
				}
			},
			UiWidgetBlueprint{
				.x = 31,
				.y = 8,
				.w = 29,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Acquired resources:",
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			UiWidgetBlueprint{
				.name = "ResourceGain",
				.x = 31,
				.y = 10,
				.w = 29,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{
					.horizontal_alignment = TextHorizontalAlignment::RIGHT
				}
			},
			UiWidgetBlueprint{
				.x = 31,
				.y = 12,
				.w = 29,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Sell requirements:",
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			UiWidgetBlueprint{
				.name = "SellRequirements",
				.x = 31,
				.y = 14,
				.w = 29,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{
					.horizontal_alignment = TextHorizontalAlignment::RIGHT
				}
			},
			UiWidgetBlueprint{
				.x = 31,
				.y = 16,
				.w = 29,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "Sell benefits:",
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			UiWidgetBlueprint{
				.name = "SellBenefits",
				.x = 31,
				.y = 18,
				.w = 29,
				.h = 2,
				.border_width = 0,
				.variant = TextBlueprint{
					.horizontal_alignment = TextHorizontalAlignment::RIGHT
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 21,
				.w = 59,
				.h = 1,
				.border_width = 0,
				.variant = TextBlueprint{
					.text = "CO = Canal Era Only   RRO = Railroad Era Only   ND = No Develop",
					.horizontal_alignment = TextHorizontalAlignment::LEFT
				}
			},
			UiWidgetBlueprint{
				.x = 1,
				.y = 23,
				.w = 59,
				.h = 2,
				.variant = TextBlueprint{
					.text = "OK",
					.onAction = [](const Text& self) -> UiAction {
						// Find the other blueprint
						if (auto* tile_selection = self.Parent().FindWidget<TextSelection>("TileLevelSelection")) {
							if (auto selections = tile_selection->GetSelectedOptions(); not selections.empty()) {
								auto item_type = static_cast<m2g::pb::ItemType>(I(selections[0]));
								return MakeReturnAction<m2g::pb::ItemType>(item_type);
							}
						}
						return MakeReturnAction();
					}
				}
			}
		}
	};
}

std::optional<m2g::pb::ItemType> ask_for_tile_selection(m2g::pb::ItemType exclude_tile) {
	LOG_INFO("Asking player to select a tile...");

	std::optional<m2g::pb::ItemType> selected_tile;
	UiPanel::create_and_run_blocking(std::make_unique<UiPanelBlueprint>(generate_tiles_window("Select tile to develop", exclude_tile)),
			tiles_window_ratio(), M2_GAME.DrawGameToTexture(M2_LEVEL.Camera()->position))
			.IfQuit([] { M2_GAME.quit = true; })
			.IfVoidReturn([&]() { LOG_INFO("Tile selection cancelled"); })
			.IfReturn<m2g::pb::ItemType>([&selected_tile](auto picked_tile) {
				LOG_INFO("Tile selected", m2g::pb::ItemType_Name(picked_tile));
				selected_tile = picked_tile;
			});

	return selected_tile;
}

std::string GetIndustryTileBuildRequirementsString(const IndustryTile industryTileType) {
	const auto& industryTile = M2_GAME.GetNamedItem(industryTileType);
	std::string build_requirements = "£" + m2::ToString(RoundI(industryTile.GetAttribute(m2g::pb::MONEY_COST)));
	if (const auto coal_cost = RoundI(industryTile.GetAttribute(m2g::pb::COAL_COST))) {
		build_requirements += ", " + m2::ToString(coal_cost) + " Coal";
	}
	if (const auto iron_cost = RoundI(industryTile.GetAttribute(m2g::pb::IRON_COST))) {
		build_requirements += ", " + m2::ToString(iron_cost) + " Iron";
	}
	return build_requirements;
}
std::string GetIndustryTileResourceGainString(const IndustryTile industryTileType) {
	const auto& industryTile = M2_GAME.GetNamedItem(industryTileType);
	std::string resource_gain;
	if (const auto coal_gain = RoundI(industryTile.GetAttribute(m2g::pb::COAL_BONUS))) {
		resource_gain = m2::ToString(coal_gain) + " Coal";
	} else if (const auto iron_gain = RoundI(industryTile.GetAttribute(m2g::pb::IRON_BONUS))) {
		resource_gain = m2::ToString(iron_gain) + " Iron";
	} else if (const auto canal_era_beer_gain = RoundI(industryTile.GetAttribute(m2g::pb::BEER_BONUS_FIRST_ERA))) {
		resource_gain = m2::ToString(canal_era_beer_gain);
		const auto railroad_era_beer_gain = RoundI(industryTile.GetAttribute(m2g::pb::BEER_BONUS_SECOND_ERA));
		if (railroad_era_beer_gain != canal_era_beer_gain) {
			resource_gain += "/" + m2::ToString(railroad_era_beer_gain);
		}
		resource_gain += " Beer";
	}
	return resource_gain;
}
std::string GetIndustryTileSellRequirementsString(const IndustryTile industryTileType) {
	const auto& industryTile = M2_GAME.GetNamedItem(industryTileType);
	if (auto sell_beer_cost = RoundI(industryTile.GetAttribute(m2g::pb::BEER_COST))) {
		return m2::ToString(sell_beer_cost) + " Beer";
	}
	return {};
}
std::string GetIndustryTileSellBenefitsString(const IndustryTile industryTileType) {
	const auto& industryTile = M2_GAME.GetNamedItem(industryTileType);
	std::string sell_benefits = m2::ToString(RoundI(industryTile.GetAttribute(m2g::pb::VICTORY_POINTS_BONUS))) + " Points, ";
	sell_benefits += m2::ToString(RoundI(industryTile.GetAttribute(m2g::pb::INCOME_POINTS_BONUS))) + " Income, ";
	sell_benefits += m2::ToString(RoundI(industryTile.GetAttribute(m2g::pb::LINK_BONUS))) + " Link";
	return sell_benefits;
}
