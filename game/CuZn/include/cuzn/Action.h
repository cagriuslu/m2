#pragma once
#include <m2/component/Character.h>
#include <m2/M2.h>
#include <m2g_Network.pb.h>

/// Try to handle the client command while the server is in liquidation state. If expected is returned, the liquidation
/// was successful. This doesn't mean the liquidation was finished, only that this liquidation action was successful.
m2::void_expected HandleActionWhileLiquidating(m2::Character& turnHolderCharacter, const m2g::pb::TurnBasedClientCommand& clientCommand);

/// Try to handle the client command while the server is not in liquidation state. If expected is returned, the return
/// value contains the money spent by the turn holder.
m2::expected<int> HandleActionWhileNotLiquidating(m2::Character& turnHolderCharacter, const m2g::pb::TurnBasedClientCommand& clientCommand, m2g::pb::TurnBasedServerCommand::ActionNotification& actionNotification);
