#pragma once
#include <m2/Object.h>

// The market is modeled as a game object that tracks the state of the market.
// This enables the ServerUpdate mechanism to update the market state automatically.
void init_game_state_tracker(m2::Object& obj);
