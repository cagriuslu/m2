#pragma once
#include <cuzn/Detail.h>

/// Returns empty set if no coal mine is reachable. Returns one location is there's only one reachable coal mine, or
/// there are multiple coal mines but one of them is closer than the others. Returns multiple coal mines if there are
/// multiple reachable coal mines, and they are equal distance away. Coal must first be consumed from the closest
/// industry.
std::set<IndustryLocation> find_closest_connected_coal_mines_with_coal(City city, City city_2 = {});

std::optional<MerchantCity> find_connected_coal_market(City city, City city_2 = {});

/// Returns true if there's a coal on the board (including the Market)
bool is_there_coal_on_the_board();
