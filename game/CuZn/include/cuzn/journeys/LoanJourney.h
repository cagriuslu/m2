#pragma once
#include <m2/component/Character.h>
#include <cuzn/Detail.h>

m2::void_expected can_player_attempt_to_loan(m2::Character& player);

void execute_loan_journey();

bool can_player_loan(m2::Character& player, const m2g::pb::ClientCommand_LoanAction& loan_action);

Card execute_loan_action(m2::Character& player, const m2g::pb::ClientCommand_LoanAction& loan_action);
