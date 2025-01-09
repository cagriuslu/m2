#pragma once

#include <m2/ui/UiPanelBlueprint.h>
#include <m2/ui/widget/Text.h>

constexpr auto quit_button_action = [](MAYBE const m2::widget::Text& self) { return m2::MakeQuitAction(); };

m2::RGB generate_player_color(unsigned index);
std::string generate_player_name(unsigned index);
