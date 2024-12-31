#pragma once
#include <cuzn/Detail.h>
#include <m2/math/RectF.h>
#include <m2/ui/PanelBlueprint.h>

// These functions return the blueprint and associated RectF which corresponds to "relation_to_game_and_hud".

std::pair<m2::ui::PanelBlueprint,m2::RectF> GenerateBuiltIndustryLocationMouseHoverUiBlueprint(IndustryLocation);
std::pair<m2::ui::PanelBlueprint,m2::RectF> GenerateEmptyIndustryLocationMouseHoverUiBlueprint(IndustryLocation);
std::pair<m2::ui::PanelBlueprint,m2::RectF> GenerateMerchantLocationMouseHoverUiBlueprint(MerchantLocation);
std::pair<m2::ui::PanelBlueprint,m2::RectF> GenerateConnectionMouseHoverUiBlueprint(Connection);
