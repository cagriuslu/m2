#include <rpg/group/CardGroup.h>
#include <m2/M2.h>
#include <m2/Math.h>

rpg::CardGroup::CardGroup(std::initializer_list<std::pair<m2g::pb::CardType, float>>&& card_probabilities) : Group() {
	// Accumulate probabilities
	float total_prob{};
	for (const auto& card_prob : card_probabilities) {
		total_prob += card_prob.second;
	}
	// Roll the dice
	float weight = m2::RandomF() * total_prob;
	// Search for the card
	m2g::pb::CardType card{};
	float accumulator{};
	for (const auto& card_prob : card_probabilities) {
		if (weight < accumulator + card_prob.second) {
			card = card_prob.first;
			break;
		}
		accumulator += card_prob.second;
	}
	_card = card;
}

std::optional<m2g::pb::CardType> rpg::CardGroup::pop_card() {
	if (_card && m2::Random64(MemberCount()) == 0) {
		auto card = *_card;
		_card.reset();
		return card;
	}
	return {};
}
