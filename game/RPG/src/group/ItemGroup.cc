#include <rpg/group/ItemGroup.h>
#include <m2/M2.h>
#include <m2/Math.h>

rpg::ItemGroup::ItemGroup(std::initializer_list<std::pair<m2g::pb::ItemType, float>>&& item_probabilities) : Group() {
	// Accumulate probabilities
	float total_prob{};
	for (const auto& item_prob : item_probabilities) {
		total_prob += item_prob.second;
	}
	// Roll the dice
	float weight = m2::randf() * total_prob;
	// Search for the item
	m2g::pb::ItemType item{};
	float accumulator{};
	for (const auto& item_prob : item_probabilities) {
		if (weight < accumulator + item_prob.second) {
			item = item_prob.first;
			break;
		}
		accumulator += item_prob.second;
	}
	_item = item;
}

std::optional<m2g::pb::ItemType> rpg::ItemGroup::pop_item() {
	if (_item && m2::rand(member_count()) == 0) {
		auto item = *_item;
		_item.reset();
		return item;
	}
	return {};
}
