#pragma once
#include <m2/Group.h>
#include <m2g_CardType.pb.h>
#include <deque>
#include <optional>

namespace rpg {
	class CardGroup : public m2::Group {
		std::optional<m2g::pb::CardType> _card;

	public:
		CardGroup(std::initializer_list<std::pair<m2g::pb::CardType, float>>&& card_probabilities);

		std::optional<m2g::pb::CardType> pop_card();
	};
}
