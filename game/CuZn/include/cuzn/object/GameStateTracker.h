#pragma once
#include <m2/Object.h>

constexpr int COAL_MARKET_CAPACITY = 14;
constexpr int IRON_MARKET_CAPACITY = 10;
constexpr int COAL_MARKET_INITIAL_COUNT = 13;
constexpr int IRON_MARKET_INITIAL_COUNT = 8;

// The market is modeled as a game object that tracks the state of the market.
// This enables the ServerUpdate mechanism to update the market state automatically.
void init_game_state_tracker(m2::Object& obj);

// Query cost of buying the resource from the market
int market_coal_cost(int coal_count);
int market_iron_cost(int iron_count);

// Query revenue of selling the resource to the market
// first: number of items that can be sold, second: revenue of selling
std::pair<int,int> market_coal_revenue(int count);
std::pair<int,int> market_iron_revenue(int count);
