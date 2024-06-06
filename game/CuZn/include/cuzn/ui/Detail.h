#pragma once

#include <m2/ui/Blueprint.h>
#include <m2/ui/widget/Text.h>

constexpr auto quit_button_action = [](MAYBE const m2::ui::widget::Text& self) { return m2::ui::make_quit_action(); };

/// The Blueprint will be used to select a card and return it from sync UI execution.
/// If exclude_card is given, one instance of that card is excluded from the list.
/// This is used for making 2 successive card selections, to exclude the first selected card from the second selection.
m2::ui::Blueprint generate_cards_window(const std::string& msg, m2g::pb::ItemType exclude_card = m2g::pb::NO_ITEM);
m2::RectF cards_window_ratio();
std::optional<m2g::pb::ItemType> ask_for_card_selection(m2g::pb::ItemType exclude_card = m2g::pb::NO_ITEM);

std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2);

bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text);
