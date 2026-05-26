#pragma once
#include <m2/component/Character.h>
#include <cuzn/Detail.h>
#include <Network.pb.h>

m2::void_expected CanPlayerAttemptToLoan(m2::FastCharacter& player);

void ExecuteLoanJourney();

// For the server
m2::void_expected CanPlayerLoan(m2::FastCharacter& player, const m2g::pb::TurnBasedClientCommand_LoanAction& loan_action);

// For the server, returns the card to be discarded
m2g::pb::CardType ExecuteLoanAction(m2::FastCharacter& player, const m2g::pb::TurnBasedClientCommand_LoanAction& loan_action);
