#include <cuzn/detail/Liquidate.h>
#include <cuzn/detail/Income.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Log.h>

int LiquidationReturnOfFactoryCharacter(m2::Character& chr) {
	auto industry_tile_type = ToIndustryTileOfFactoryCharacter(chr);
	const auto& industry_tile_item = M2_GAME.GetNamedItem(industry_tile_type);
	auto money_cost_f = industry_tile_item.GetAttribute(m2g::pb::MONEY_COST);
	auto money_cost_i = m2::iround(money_cost_f);
	auto rounded_down_half_cost = money_cost_i >> 1;
	return rounded_down_half_cost;
}

int IsLiquidationNecessaryForPlayer(m2::Character& player_character) {
	const auto income_points = m2::iround(player_character.GetAttribute(m2g::pb::INCOME_POINTS));
	const auto income_level = IncomeLevelFromIncomePoints(income_points);
	const auto player_money = m2::iround(player_character.GetResource(m2g::pb::MONEY));
	// Check if player money would go below zero, and the player has at least one factory to sell
	if (player_money + income_level < 0 && PlayerBuiltFactoryCount(player_character)) {
		return -(player_money + income_level);
	}
	return 0;
}

std::optional<std::pair<m2g::Proxy::PlayerIndex, int>> IsLiquidationNecessary() {
	for (int i = 0; i < m2::I(M2G_PROXY.multiPlayerObjectIds.size()); ++i) {
		auto playerId = M2G_PROXY.multiPlayerObjectIds[i];
		auto& player_character = M2_LEVEL.objects[playerId].character();
		auto liquidation_amount = IsLiquidationNecessaryForPlayer(player_character);
		if (0 < liquidation_amount) {
			return std::make_pair(i, liquidation_amount);
		}
	}
	return std::nullopt;
}

namespace {
	struct LocationAndLiquidationReturnPair {
		Location location;
		int liquidation_return;
	};

	struct LocationAndLiquidationReturnPairComparator {
		bool operator()(const LocationAndLiquidationReturnPair& lhs, const LocationAndLiquidationReturnPair& rhs) const {
			return lhs.liquidation_return < rhs.liquidation_return;
		}
	};
}

m2::expected<std::pair<std::vector<m2::Object*>, int>> CanPlayerLiquidateFactories(m2::Character& player, const m2g::pb::ClientCommand_LiquidateAction& liquidate_action) {
	LOG_DEBUG("Validating liquidate command");

	// Check if the player needed a liquidation in the first place
	const auto necessaryLiquidationAmount = IsLiquidationNecessaryForPlayer(player);
	if (necessaryLiquidationAmount <= 0) {
		return m2::make_unexpected("Player didn't need liquidation");
	}

	if (liquidate_action.locations_to_sell_size() <= 0) {
		return m2::make_unexpected("LiquidateCommand doesn't contain any locations");
	}

	std::vector<m2::Object*> factoryObjects;

	// Order locations from cheap to expensive
	std::set<LocationAndLiquidationReturnPair, LocationAndLiquidationReturnPairComparator> ordered_location_and_liquidation_return_pairs;
	for (auto location_i : liquidate_action.locations_to_sell()) {
		const auto location = static_cast<Location>(location_i);
		// Search for a factory in the given location
		if (auto* factory = FindFactoryAtLocation(location)) {
			// Check if the factory belongs to the player
			if (factory->parent_id() == player.owner().id()) {
				factoryObjects.emplace_back(factory);
				const auto liquidation_return = LiquidationReturnOfFactoryCharacter(factory->character());
				ordered_location_and_liquidation_return_pairs.emplace(LocationAndLiquidationReturnPair{location, liquidation_return});
			} else {
				return m2::make_unexpected("Factory does not belong to the liquidating player: " + m2g::pb::SpriteType_Name(location));
			}
		} else {
			return m2::make_unexpected("Factory is not built for location: " + m2g::pb::SpriteType_Name(location));
		}
	}

	// Check if there are duplicates
	if (m2::I(ordered_location_and_liquidation_return_pairs.size()) != liquidate_action.locations_to_sell_size()) {
		return m2::make_unexpected("LiquidateCommand contains duplicated locations");
	}

	// Iterate the set in reverse order; expensive to cheap, and stop when enough factories are sold
	int liquidatedFactoryCount = 0;
	int cumulativeLiquidationReturn = 0;
	for (auto rit = ordered_location_and_liquidation_return_pairs.rbegin();
		rit != ordered_location_and_liquidation_return_pairs.rend() && cumulativeLiquidationReturn < necessaryLiquidationAmount;
		++rit) {
		liquidatedFactoryCount++;
		cumulativeLiquidationReturn += rit->liquidation_return;
	}

	if (liquidatedFactoryCount != liquidate_action.locations_to_sell_size()) {
		return m2::make_unexpected("Player selected more than enough liquidation locations");
	}

	return std::make_pair(std::move(factoryObjects), cumulativeLiquidationReturn);
}
