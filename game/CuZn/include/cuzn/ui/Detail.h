#pragma once

#include <m2/ui/UiPanelBlueprint.h>
#include <m2/ui/widget/Text.h>

constexpr auto quit_button_action = [](MAYBE const m2::widget::Text& self) { return m2::MakeQuitAction(); };

std::optional<m2g::pb::ItemType> ask_for_industry_selection(m2g::pb::ItemType industry_1, m2g::pb::ItemType industry_2);

bool ask_for_confirmation(const std::string& question1, const std::string& question2, const std::string& accept_text, const std::string& decline_text);
std::optional<bool> ask_for_confirmation_with_cancellation(const std::string& question, const std::string& accept_text, const std::string& decline_text);
bool ask_for_confirmation_bottom(const std::string& question, const std::string& accept_text, const std::string& decline_text, m2::sdl::TextureUniquePtr background_texture);

m2::RGB generate_player_color(unsigned index);
std::string generate_player_name(unsigned index);
