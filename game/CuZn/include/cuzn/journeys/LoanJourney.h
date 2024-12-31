#pragma once
#include <m2/component/Character.h>
#include <cuzn/Detail.h>
#include <Network.pb.h>

m2::void_expected CanPlayerAttemptToLoan(m2::Character& player);

void ExecuteLoanJourney();

// For the server
bool CanPlayerLoan(m2::Character& player, const m2g::pb::ClientCommand_LoanAction& loan_action);

// For the server, returns the card to be discarded
Card ExecuteLoanAction(m2::Character& player, const m2g::pb::ClientCommand_LoanAction& loan_action);
