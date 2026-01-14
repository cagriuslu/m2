#include <rpg/UseCard.h>


bool rpg::UseCard(m2::Character& chr, const m2::Card& card, const float resource_multiplier) {
	// Check if costs can be paid
	if (card.GetCostCount() == 1) {
		const auto cost = card.GetCostByIndex(0);
		const auto adjusted_cost_amount = cost.second * resource_multiplier;
		if (0.0f < adjusted_cost_amount && chr.GetResource(cost.first) < adjusted_cost_amount) {
			return false;
		}
	} else if (1 < card.GetCostCount()) {
		// Merge costs
		auto resource_count = m2::pb::enum_value_count<m2g::pb::ResourceType>();
		auto* merged_costs = static_cast<float*>(alloca(resource_count * sizeof(float)));
		memset(merged_costs, 0, resource_count * sizeof(float));
		for (size_t i = 0; i < card.GetCostCount(); ++i) {
			const auto cost = card.GetCostByIndex(i);
			merged_costs[m2::pb::enum_index(cost.first)] += cost.second * resource_multiplier;
		}
		// Check if all costs are covered
		for (int i = 0; i < resource_count; ++i) {
			if (0.0f < merged_costs[i] && chr.GetResource(m2::pb::enum_value<m2g::pb::ResourceType>(i)) < merged_costs[i]) {
				return false;
			}
		}
	}

	// Pay the costs
	for (size_t i = 0; i < card.GetCostCount(); ++i) {
		const auto cost = card.GetCostByIndex(i);
		chr.RemoveResource(cost.first, cost.second * resource_multiplier);
	}
	// Get the benefits
	for (size_t i = 0; i < card.GetBenefitCount(); ++i) {
		const auto benefit = card.GetBenefitByIndex(i);
		chr.AddResource(benefit.first, benefit.second * resource_multiplier);
	}
	return true;
}
