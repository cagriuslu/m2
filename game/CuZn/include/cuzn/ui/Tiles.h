#pragma once
#include <m2/ui/PanelBlueprint.h>
#include <m2/RectF.h>
#include <m2g_ItemType.pb.h>

m2::RectF tiles_window_ratio();

m2::ui::PanelBlueprint generate_tiles_window(const std::string& msg, m2g::pb::ItemType exclude_tile = m2g::pb::NO_ITEM);

std::optional<m2g::pb::ItemType> ask_for_tile_selection(m2g::pb::ItemType exclude_tile = m2g::pb::NO_ITEM);
