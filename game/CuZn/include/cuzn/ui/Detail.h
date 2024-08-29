#pragma once

#include <m2/ui/Blueprint.h>
#include <m2/ui/widget/Text.h>

constexpr auto quit_button_action = [](MAYBE const m2::ui::widget::Text& self) { return m2::ui::make_quit_action(); };

std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2);

bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text);
bool ask_for_confirmation_bottom(const std::string& question, const std::string& accept_text, const std::string& decline_text, m2::sdl::TextureUniquePtr background_texture);

void display_blocking_message(const std::string&);

m2::RGB generate_player_color(unsigned index);
std::string generate_player_name(unsigned index);
