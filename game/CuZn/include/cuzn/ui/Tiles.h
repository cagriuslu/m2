#pragma once
#include <cuzn/Detail.h>
#include <m2/ui/UiPanelBlueprint.h>
#include <m2/math/RectF.h>
#include <m2g_CardType.pb.h>

m2::RectF tiles_window_ratio();

m2::UiPanelBlueprint generate_tiles_window(const std::string& msg, m2g::pb::CardType exclude_tile = m2g::pb::NO_CARD);

/// Returns the selected tile unless the selection is cancelled, in which case nullopt will be returned.
std::optional<m2g::pb::CardType> ask_for_tile_selection(m2g::pb::CardType exclude_tile = m2g::pb::NO_CARD);

std::string GetIndustryTileBuildRequirementsString(IndustryTile);
std::string GetIndustryTileResourceGainString(IndustryTile);
std::string GetIndustryTileSellRequirementsString(IndustryTile);
std::string GetIndustryTileSellBenefitsString(IndustryTile);
