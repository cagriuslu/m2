#pragma once
#include <m2/ui/UiPanelBlueprint.h>
#include <m2/math/RectF.h>
#include <m2g_ItemType.pb.h>

m2::RectF cards_window_ratio();
m2::RectF cards_panel_ratio();

/// The UiPanelBlueprint will be used to select a card and return it from sync UI execution.
/// If exclude_card is given, one instance of that card is excluded from the list.
/// This is used for making 2 successive card selections, to exclude the first selected card from the second selection.
m2::UiPanelBlueprint generate_cards_window(const std::string& msg, m2g::pb::ItemType exclude_card_1 = m2g::pb::NO_ITEM,
	m2g::pb::ItemType exclude_card_2 = m2g::pb::NO_ITEM, bool blocking_window = false);

std::optional<m2g::pb::ItemType> ask_for_card_selection(m2g::pb::ItemType exclude_card_1 = m2g::pb::NO_ITEM,
	m2g::pb::ItemType exclude_card_2 = m2g::pb::NO_ITEM);
