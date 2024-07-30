#pragma once
#include <cuzn/Detail.h>

// Beer can be consumed from player's breweries, other players' connected breweries, or the merchant that's being sold to.
std::set<Location> find_breweries_with_beer(m2::Character& player, City city, std::optional<MerchantLocation> selling_to);
