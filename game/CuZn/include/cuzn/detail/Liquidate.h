#pragma once
#include <m2/component/Character.h>
#include <m2/Meta.h>
#include <m2g_Network.pb.h>
#include <m2g/Proxy.h>

int LiquidationReturnOfFactoryCharacter(m2::Character& chr);

/// Returns positive integer if liquidation is necessary. The necessary amount of liquidation is returned.
/// Returns zero if not necessary.
int IsLiquidationNecessaryForPlayer(m2::Character& player);
std::optional<std::pair<m2g::Proxy::PlayerIndex, int>> IsLiquidationNecessary();

/// If validation is successful, returns a vector of factory objects that can be deleted, and total money earned from
/// the sale. Otherwise, returns the reason for validation failure.
m2::expected<std::pair<std::vector<m2::Object*>, int>> CanPlayerLiquidateFactories(m2::Character& player, const m2g::pb::ClientCommand_LiquidateAction&);
