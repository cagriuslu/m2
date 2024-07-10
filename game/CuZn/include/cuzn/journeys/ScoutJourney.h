#pragma once
#include <m2/component/Character.h>
#include <cuzn/Detail.h>

m2::void_expected can_player_attempt_to_scout(m2::Character& player);

void execute_scout_journey();

// For the server
bool can_player_scout(m2::Character& player, const m2g::pb::ClientCommand_ScoutAction& scout_action);

// For the server, returns the card to be discarded
Card execute_scout_action(m2::Character& player, const m2g::pb::ClientCommand_ScoutAction& scout_action);
