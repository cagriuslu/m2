#pragma once
#include <cuzn/Detail.h>

// Coal must first be consumed from the closest industry
std::vector<IndustryLocation> find_closest_connected_coal_mines_with_coal(City city, City city_2 = {});

std::optional<MerchantCity> find_connected_coal_market(City city, City city_2 = {});
