#pragma once

#include <m2/ui/Blueprint.h>
#include <m2/ui/widget/Text.h>

constexpr auto quit_button_action = [](MAYBE const m2::ui::widget::Text& self) { return m2::ui::make_quit_action(); };

/// The Blueprint will be used to select a card and return it from sync UI execution.
/// If exclude_card is given, one instance of that card is excluded from the list.
/// This is used for making 2 successive card selections, to exclude the first selected card from the second selection.
m2::ui::Blueprint generate_cards_window(const std::string& msg, m2g::pb::ItemType exclude_card_1 = m2g::pb::NO_ITEM, m2g::pb::ItemType exclude_card_2 = m2g::pb::NO_ITEM);
m2::RectF cards_window_ratio();
std::optional<m2g::pb::ItemType> ask_for_card_selection(m2g::pb::ItemType exclude_card_1 = m2g::pb::NO_ITEM, m2g::pb::ItemType exclude_card_2 = m2g::pb::NO_ITEM);

m2::ui::Blueprint generate_tiles_window(const std::string& msg, m2g::pb::ItemType exclude_tile = m2g::pb::NO_ITEM);
m2::RectF tiles_window_ratio();
std::optional<m2g::pb::ItemType> ask_for_tile_selection(m2g::pb::ItemType exclude_tile = m2g::pb::NO_ITEM);

std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2);

bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text);
bool ask_for_confirmation_bottom(const std::string& question, const std::string& accept_text, const std::string& decline_text, m2::sdl::TextureUniquePtr background_texture);

void display_blocking_message(const std::string& line1, const std::string& line2);

m2::RGB generate_player_color(unsigned index);
std::string generate_player_name(unsigned index);
