#include <cuzn/ui/MouseHover.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Merchant.h>
#include <cuzn/object/Road.h>
#include <cuzn/ui/Tiles.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2;

std::pair<UiPanelBlueprint,RectF> GenerateBuiltIndustryLocationMouseHoverUiBlueprint(const IndustryLocation loc) {
	const auto* factory = FindFactoryAtLocation(loc);
	if (not factory) {
		throw M2_ERROR("Factory not found");
	}

	const auto industryType = ToIndustryOfFactoryCharacter(factory->character());
	const auto& industryCard = M2_GAME.GetNamedItem(industryType);
	const auto industryTileType = ToIndustryTileOfFactoryCharacter(factory->character());
	const auto& industryTile = M2_GAME.GetNamedItem(industryTileType);
	return std::make_pair(
			UiPanelBlueprint{
				.name = "BuiltIndustryLocationHover",
				.w = 4, .h = 4,
				.border_width = 0.001f,
				.background_color = {0, 0, 0, 255},
				.widgets = {
					UiWidgetBlueprint{
						.x = 0, .y = 0, .w = 4, .h = 1,
						.border_width = 0.001f,
						.background_color = {0, 0, 127, 255},
						.variant = widget::TextBlueprint {
							.text = industryCard.in_game_name(),
							.wrapped_font_size_in_units = 0.75f
						}
					},
					UiWidgetBlueprint{
						.x = 0, .y = 1, .w = 4, .h = 1,
						.border_width = 0.0f,
						.background_color = {0, 0, 0, 255},
						.variant = widget::TextBlueprint {
							.text = "Level     : " + industryTile.in_game_name(),
							.horizontal_alignment = TextHorizontalAlignment::LEFT,
							.wrapped_font_size_in_units = 0.75f
						}
					},
					UiWidgetBlueprint{
						.x = 0, .y = 2, .w = 4, .h = 1,
						.border_width = 0.0f,
						.background_color = {0, 0, 0, 255},
						.variant = widget::TextBlueprint {
							.text = "Sell Reqs : " + GetIndustryTileSellRequirementsString(industryTileType),
							.horizontal_alignment = TextHorizontalAlignment::LEFT,
							.wrapped_font_size_in_units = 0.75f
						}
					},
					UiWidgetBlueprint{
						.x = 0, .y = 3, .w = 4, .h = 1,
						.border_width = 0.0f,
						.background_color = {0, 0, 0, 255},
						.variant = widget::TextBlueprint {
							.text = "Sell Benft: " + GetIndustryTileSellBenefitsString(industryTileType),
							.horizontal_alignment = TextHorizontalAlignment::LEFT,
							.wrapped_font_size_in_units = 0.75f
						}
					}
				}
			},
			RectF{0.0f, 0.0f, 0.33f, 0.1270f});
}

std::pair<UiPanelBlueprint,RectF> GenerateEmptyIndustryLocationMouseHoverUiBlueprint(const IndustryLocation loc) {
	const auto industries = industries_on_location(loc);
	// There should at most be 2 industries
	if (industries.empty() || 2 < industries.size()) {
		throw M2_ERROR("Invalid number of industries");
	}

	if (industries.size() == 1) {
		const auto& industryCard = M2_GAME.GetNamedItem(industries[0]);
		const auto nextIndustryTileType = PlayerNextIndustryTileOfIndustry(M2_PLAYER.character(), industries[0]);
		const auto* nextIndustryTile = nextIndustryTileType ? &M2_GAME.GetNamedItem(*nextIndustryTileType) : nullptr;
		return std::make_pair(
				UiPanelBlueprint{
					.name = "EmptyIndustryLocationHover",
					.w = 4, .h = 7,
					.border_width = 0.001f,
					.background_color = {0, 0, 0, 255},
					.widgets = {
						UiWidgetBlueprint{
							.x = 0, .y = 0, .w = 4, .h = 1,
							.border_width = 0.001f,
							.background_color = {0, 0, 127, 255},
							.variant = widget::TextBlueprint {
								.text = industryCard.in_game_name(),
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 1, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile
										? "Your next " + industryCard.in_game_name() + " tile"
										: industryCard.in_game_name() + " tile not found",
								.horizontal_alignment = TextHorizontalAlignment::CENTER,
								.wrapped_font_size_in_units = 0.75f,
								.color = {127, 127, 127}
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 2, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile
										? "Level     : " + nextIndustryTile->in_game_name()
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 3, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile
										? "Build Reqs: " + GetIndustryTileBuildRequirementsString(*nextIndustryTileType)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 4, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile
										? "Build Gain: " + GetIndustryTileResourceGainString(*nextIndustryTileType)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 5, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile
										? "Sell Reqs : " + GetIndustryTileSellRequirementsString(*nextIndustryTileType)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 6, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile
										? "Sell Benft: " + GetIndustryTileSellBenefitsString(*nextIndustryTileType)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						}
					}
				},
				RectF{0.0f, 0.0f, 0.33f, 0.2222f});
	} else { // industries.size() == 2
		const auto& industryCard1 = M2_GAME.GetNamedItem(industries[0]);
		const auto& industryCard2 = M2_GAME.GetNamedItem(industries[1]);
		const auto nextIndustryTileType1 = PlayerNextIndustryTileOfIndustry(M2_PLAYER.character(), industries[0]);
		const auto nextIndustryTileType2 = PlayerNextIndustryTileOfIndustry(M2_PLAYER.character(), industries[1]);
		const auto* nextIndustryTile1 = nextIndustryTileType1 ? &M2_GAME.GetNamedItem(*nextIndustryTileType1) : nullptr;
		const auto* nextIndustryTile2 = nextIndustryTileType2 ? &M2_GAME.GetNamedItem(*nextIndustryTileType2) : nullptr;
		return std::make_pair(
				UiPanelBlueprint{
					.name = "EmptyIndustryLocationHover2",
					.w = 4, .h = 14,
					.border_width = 0.001f,
					.background_color = {0, 0, 0, 255},
					.widgets = {
						// First tile
						UiWidgetBlueprint{
							.x = 0, .y = 0, .w = 4, .h = 1,
							.border_width = 0.001f,
							.background_color = {0, 0, 127, 255},
							.variant = widget::TextBlueprint {
								.text = industryCard1.in_game_name(),
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 1, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile1
										? "Your next " + industryCard1.in_game_name() + " tile"
										: industryCard1.in_game_name() + " tile not found",
								.horizontal_alignment = TextHorizontalAlignment::CENTER,
								.wrapped_font_size_in_units = 0.75f,
								.color = {127, 127, 127}
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 2, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile1
										? "Level     : " + nextIndustryTile1->in_game_name()
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 3, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile1
										? "Build Reqs: " + GetIndustryTileBuildRequirementsString(*nextIndustryTileType1)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 4, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile1
										? "Build Gain: " + GetIndustryTileResourceGainString(*nextIndustryTileType1)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 5, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile1
										? "Sell Reqs : " + GetIndustryTileSellRequirementsString(*nextIndustryTileType1)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 6, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile1
										? "Sell Benft: " + GetIndustryTileSellBenefitsString(*nextIndustryTileType1)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						// Second tile
						UiWidgetBlueprint{
							.x = 0, .y = 7, .w = 4, .h = 1,
							.border_width = 0.001f,
							.background_color = {0, 0, 127, 255},
							.variant = widget::TextBlueprint {
								.text = industryCard2.in_game_name(),
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 8, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile2
										? "Your next " + industryCard2.in_game_name() + " tile"
										: industryCard2.in_game_name() + " tile not found",
								.horizontal_alignment = TextHorizontalAlignment::CENTER,
								.wrapped_font_size_in_units = 0.75f,
								.color = {127, 127, 127}
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 9, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile2
										? "Level     : " + nextIndustryTile2->in_game_name()
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 10, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile2
										? "Build Reqs: " + GetIndustryTileBuildRequirementsString(*nextIndustryTileType2)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 11, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile2
										? "Build Gain: " + GetIndustryTileResourceGainString(*nextIndustryTileType2)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 12, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile2
										? "Sell Reqs : " + GetIndustryTileSellRequirementsString(*nextIndustryTileType2)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						},
						UiWidgetBlueprint{
							.x = 0, .y = 13, .w = 4, .h = 1,
							.border_width = 0.0f,
							.background_color = {0, 0, 0, 255},
							.variant = widget::TextBlueprint {
								.text = nextIndustryTile2
										? "Sell Benft: " + GetIndustryTileSellBenefitsString(*nextIndustryTileType2)
										: "",
								.horizontal_alignment = TextHorizontalAlignment::LEFT,
								.wrapped_font_size_in_units = 0.75f
							}
						}
					}
				},
				RectF{0.0f, 0.0f, 0.33f, 0.4444f});
	}
}

std::pair<UiPanelBlueprint,RectF> GenerateMerchantLocationMouseHoverUiBlueprint(const MerchantLocation loc) {
	const auto* merchant = find_merchant_at_location(loc);
	const std::string merchantBenefit = merchant->character().has_resource(m2g::pb::BEER_BARREL_COUNT)
			? "Available" : "Exhausted";
	const auto merchantBenefitBackgroundColor = merchant->character().has_resource(m2g::pb::BEER_BARREL_COUNT)
			? SDL_Color{0, 0, 0, 255} : SDL_Color{180, 0, 0, 255};

	return std::make_pair(
			UiPanelBlueprint{
				.name = "MerchantLocationHover",
				.w = 4, .h = 3,
				.border_width = 0.001f,
				.background_color = {0, 0, 0, 255},
				.widgets = {
					UiWidgetBlueprint{
						.x = 0, .y = 0, .w = 4, .h = 1,
						.border_width = 0.001f,
						.background_color = {0, 0, 127, 255},
						.variant = widget::TextBlueprint {
							.text = "Merchant",
							.wrapped_font_size_in_units = 0.75f
						}
					},
					UiWidgetBlueprint{
						.x = 0, .y = 1, .w = 4, .h = 1,
						.border_width = 0.0f,
						.background_color = {0, 0, 0, 255},
						.variant = widget::TextBlueprint {
							.text = "Total Link Count: 2",
							.horizontal_alignment = TextHorizontalAlignment::LEFT,
							.wrapped_font_size_in_units = 0.75f
						}
					},

					UiWidgetBlueprint{
						.x = 0, .y = 2, .w = 4, .h = 1,
						.border_width = 0.0f,
						.background_color = merchantBenefitBackgroundColor,
						.variant = widget::TextBlueprint {
							.text = "Merchant Benefit: " + merchantBenefit,
							.horizontal_alignment = TextHorizontalAlignment::LEFT,
							.wrapped_font_size_in_units = 0.75f
						}
					}
				}
			},
			RectF{0.0f, 0.0f, 0.33f, 0.09525f});
}

std::pair<UiPanelBlueprint,RectF> GenerateConnectionMouseHoverUiBlueprint(Connection loc) {
	const auto* road = FindRoadAtLocation(loc);
	return std::make_pair(
			UiPanelBlueprint{
				.name = "ConnectionHover",
				.w = 4, .h = 2,
				.border_width = 0.001f,
				.background_color = {0, 0, 0, 255},
				.widgets = {
					UiWidgetBlueprint{
						.x = 0, .y = 0, .w = 4, .h = 1,
						.border_width = 0.001f,
						.background_color = {0, 0, 127, 255},
						.variant = widget::TextBlueprint {
							.text = "Connection",
							.wrapped_font_size_in_units = 0.75f
						}
					},
					UiWidgetBlueprint{
						.x = 0, .y = 1, .w = 4, .h = 1,
						.border_width = 0.0f,
						.background_color = {0, 0, 0, 255},
						.variant = widget::TextBlueprint {
							.text = road
									? "Current Link Count: " + ToString(LinkCountOfRoadCharacter(road->character()))
									: "Estimated Link Count: " + ToString(LinkCountOfConnectionLocation(loc)),
							.horizontal_alignment = TextHorizontalAlignment::LEFT,
							.wrapped_font_size_in_units = 0.75f
						}
					}
				}
			},
			RectF{0.0f, 0.0f, 0.33f, 0.0635f});
}
