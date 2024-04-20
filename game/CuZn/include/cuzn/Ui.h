#pragma once
#include <m2/ui/Blueprint.h>

namespace cuzn {


	/// The Blueprint will be used to select a card and return it from sync UI execution.
	/// If exclude_card is given, one instance of that card is excluded from the list.
	/// This is used for making 2 successive card selections, to exclude the first selected card from the second selection.
	m2::ui::Blueprint generate_cards_window(bool has_return_button, m2g::pb::ItemType exclude_card = m2g::pb::NO_ITEM);
	std::optional<m2g::pb::ItemType> ask_for_card_selection(m2g::pb::ItemType exclude_card = m2g::pb::NO_ITEM);

	m2::ui::Blueprint generate_industry_selection_window(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2);
	std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2);
	// TODO replace with ask_for_confirmation^^

	m2::ui::Blueprint generate_build_confirmation(m2g::pb::ItemType card, m2g::pb::ItemType city, m2g::pb::ItemType industry);
	// TODO replace with ask_for_confirmation^^

	bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text);
}
