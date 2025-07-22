#pragma once
#include <m2/component/Character.h>
#include <cuzn/Detail.h>
#include <Network.pb.h>

m2::void_expected CanPlayerAttemptToScout(m2::Character& player);

void ExecuteScoutJourney();

// For the server
m2::void_expected CanPlayerScout(m2::Character& player, const m2g::pb::TurnBasedClientCommand_ScoutAction& scout_action);

// For the server, returns the card to be discarded
Card ExecuteScoutAction(m2::Character& player, const m2g::pb::TurnBasedClientCommand_ScoutAction& scout_action);
