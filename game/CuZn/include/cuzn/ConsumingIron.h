#pragma once
#include <cuzn/Detail.h>

// Iron can be consumed from any location in the map
std::set<IndustryLocation> find_iron_industries_with_iron();

/// Returns true if there's iron on the board (including the Market)
bool is_there_iron_on_the_board();
