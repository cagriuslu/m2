#include <m2/Card.h>
#include <m2/protobuf/Detail.h>
#include <m2/Error.h>
#include <m2/Game.h>
#include <utility>

m2::Card::Card(pb::Card card) : _card(std::move(card)) {
	for (const auto& constant : _card.constants()) {
		_constants[pb::enum_index(constant.type())] = IVFE{constant.ivfe()};
	}
}

const m2::Card& m2::ToCard(m2g::pb::CardType card_type) {
	return M2_GAME.GetCard(card_type);
}
