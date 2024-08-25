#pragma once
#include <utility>

constexpr int COAL_MARKET_CAPACITY = 14;
constexpr int IRON_MARKET_CAPACITY = 10;
constexpr int COAL_MARKET_INITIAL_COUNT = 13;
constexpr int IRON_MARKET_INITIAL_COUNT = 8;

// Returns the cost of buying `to_buy` number of items from the market
int calculate_cost(int capacity, int current_resource_count, int to_buy);

// Returns the number of items that can be solved, and the total revenue of doing so.
std::pair<int,int> calculate_revenue(int capacity, int current_resource_count, int count);
