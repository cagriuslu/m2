#pragma once
#include <cuzn/Detail.h>

std::vector<IndustryLocation> find_closest_connected_coal_mines_with_coal(City city);

std::optional<MerchantCity> find_connected_coal_market_with_coal(City city);
