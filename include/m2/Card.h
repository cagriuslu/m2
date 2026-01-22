#pragma once
#include "protobuf/Detail.h"
#include <m2/math/IVFE.h>
#include <Character.pb.h>
#include <vector>
#include <string>

namespace m2 {
	class Card {
		pb::Card _card;
		std::vector<IVFE> _constants = std::vector<IVFE>(pb::enum_value_count<m2g::pb::ConstantType>());

	public:
		Card() = default;
		explicit Card(pb::Card card);
		// Prevent accidental copying
		Card(const Card& other) = delete;
		Card& operator=(const Card& other) = delete;
		// Move allowed
		Card(Card&& other) = default;
		Card& operator=(Card&& other) = default;

		[[nodiscard]] m2g::pb::CardType Type() const { return _card.type(); }
		[[nodiscard]] m2g::pb::CardCategory Category() const { return _card.category(); }
		[[nodiscard]] IVFE GetConstant(const m2g::pb::ConstantType c) const { return _constants[pb::enum_index(c)]; }
		[[nodiscard]] m2g::pb::SpriteType GameSprite() const { return _card.game_sprite(); }
		[[nodiscard]] m2g::pb::SpriteType UiSprite() const { return _card.ui_sprite(); }
		[[nodiscard]] const std::string& in_game_name() const { return _card.in_game_name(); }
	};

	// Transformers
	const Card& ToCard(m2g::pb::CardType card_type);
}
