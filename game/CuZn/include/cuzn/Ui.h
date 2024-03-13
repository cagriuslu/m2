#pragma once
#include <m2/ui/Blueprint.h>

namespace cuzn {
	/// The Blueprint will be used to select a card and return it from sync UI execution.
	m2::ui::Blueprint generate_cards_window(bool has_return_button);

	m2::ui::Blueprint generate_industry_selection_window(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2);

	m2::ui::Blueprint generate_build_confirmation(m2g::pb::ItemType card, m2g::pb::ItemType city, m2g::pb::ItemType industry);
}
