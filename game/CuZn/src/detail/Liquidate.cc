#include <cuzn/detail/Liquidate.h>
#include <cuzn/detail/Income.h>
#include <m2/Game.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/HumanPlayer.h>
#include <m2/Log.h>

int liquidation_return_of_factory_character(m2::Character& chr) {
	auto industry_tile_type = to_industry_tile_of_factory_character(chr);
	const auto& industry_tile_item = M2_GAME.GetNamedItem(industry_tile_type);
	auto money_cost_f = industry_tile_item.get_attribute(m2g::pb::MONEY_COST);
	auto money_cost_i = m2::iround(money_cost_f);
	auto rounded_down_half_cost = money_cost_i >> 1;
	return rounded_down_half_cost;
}

int is_liquidation_necessary_for_player(m2::Character& player_character) {
	auto income_points = m2::iround(player_character.get_attribute(m2g::pb::INCOME_POINTS));
	auto income_level = income_level_from_income_points(income_points);
	auto player_money = m2::iround(player_character.get_resource(m2g::pb::MONEY));
	// Check if player money would go below zero, and the player has at least one factory to sell
	if (player_money + income_level < 0 && player_built_factory_count(player_character)) {
		return -(player_money + income_level);
	}
	return 0;
}

std::optional<std::pair<m2g::Proxy::PlayerIndex, int>> is_liquidation_necessary() {
	for (int i = 0; i < m2::I(M2G_PROXY.multi_player_object_ids.size()); ++i) {
		auto player_id = M2G_PROXY.multi_player_object_ids[i];
		auto& player_character = M2_LEVEL.objects[player_id].character();
		auto liquidation_amount = is_liquidation_necessary_for_player(player_character);
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

m2::expected<std::pair<std::vector<m2::Object*>, int>> can_player_liquidate_factories(m2::Character& player, const m2g::pb::ClientCommand_LiquidateAction& liquidate_action) {
	LOG_DEBUG("Validating liquidate command");

	// Check if the player needed a liquidation in the first place
	auto necessary_liquidation_amount = is_liquidation_necessary_for_player(player);
	if (necessary_liquidation_amount <= 0) {
		return m2::make_unexpected("Player didn't need liquidation");
	}

	if (liquidate_action.locations_to_sell_size() <= 0) {
		return m2::make_unexpected("LiquidateCommand doesn't contain any locations");
	}

	std::vector<m2::Object*> factory_objects;

	// Order locations from cheap to expensive
	std::set<LocationAndLiquidationReturnPair, LocationAndLiquidationReturnPairComparator> ordered_location_and_liquidation_return_pairs;
	for (auto location_i : liquidate_action.locations_to_sell()) {
		auto location = static_cast<Location>(location_i);
		// Search for a factory in the given location
		if (auto* factory = find_factory_at_location(location)) {
			// Check if the factory belongs to the player
			if (factory->parent_id() == player.owner().id()) {
				factory_objects.emplace_back(factory);
				auto liquidation_return = liquidation_return_of_factory_character(factory->character());
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
	int liquidated_factory_count = 0;
	int cumulative_liquidation_return = 0;
	for (auto rit = ordered_location_and_liquidation_return_pairs.rbegin();
		rit != ordered_location_and_liquidation_return_pairs.rend() && cumulative_liquidation_return < necessary_liquidation_amount;
		++rit) {
		liquidated_factory_count++;
		cumulative_liquidation_return += rit->liquidation_return;
	}

	if (liquidated_factory_count != liquidate_action.locations_to_sell_size()) {
		return m2::make_unexpected("Player selected more than enough liquidation locations");
	}

	return std::make_pair(std::move(factory_objects), cumulative_liquidation_return);
}
