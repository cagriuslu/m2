#pragma once
#include <m2/Object.h>

// The market is modeled as a game object that tracks the state of the market.
// This enables the ServerUpdate mechanism to update the market state automatically.
void init_market(m2::Object& obj);

// Query cost of buying the resource from the market
int market_coal_cost(int coal_count);
int market_iron_cost(int iron_count);

// Query revenue of selling the resource to the market
// first: number of items that can be sold, second: revenue of selling
std::pair<int,int> market_coal_revenue(int count);
std::pair<int,int> market_iron_revenue(int count);

