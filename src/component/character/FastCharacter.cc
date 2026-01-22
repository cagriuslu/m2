#include <m2/component/character/FastCharacter.h>
#include <m2/Game.h>

using namespace m2;

FastCharacter::FastCharacter(uint64_t object_id) : Character(object_id) {}
int32_t FastCharacter::Hash(const int32_t initialValue) const {
	if constexpr (not GAME_IS_DETERMINISTIC) {
		// ReSharper disable once CppDFAUnreachableCode
		throw M2_ERROR("Game is not deterministic");
	}
	// ReSharper disable once CppDFAUnreachableCode
	auto hash = initialValue;
	for (const auto* card : _cards) {
		if (card) {
			hash = HashI(card->Type(), hash);
		}
	}
	for (const auto& variable : _variables) {
		if (variable && variable.IsInt()) {
			hash = HashI(variable.UnsafeGetInt(), hash);
		} else if (variable && variable.IsFE()) {
			hash = HashI(ToRawValue(variable.UnsafeGetFE()), hash);
		}
	}
	return hash;
}
void FastCharacter::Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const {
	for (const auto* card : _cards) {
		if (card) {
			objDesc.add_cards(card->Type());
		}
	}
	for (int i = 0; i < pb::enum_value_count<m2g::pb::VariableType>(); ++i) {
		if (_variables[i]) {
			auto* var = objDesc.add_variables();
			var->set_type(static_cast<m2g::pb::VariableType>(i));
			var->mutable_ivfe()->CopyFrom(static_cast<pb::IVFE>(_variables[i]));
		}
	}
}
void FastCharacter::Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
	_cards.clear();
	_variables = std::vector<IVFE>(pb::enum_value_count<m2g::pb::VariableType>());
	for (const auto& card : objDesc.cards()) {
		_cards.emplace_back(&M2_GAME.GetCard(static_cast<m2g::pb::CardType>(card)));
	}
	for (const auto& variable : objDesc.variables()) {
		_variables[pb::enum_index(variable.type())] = IVFE{variable.ivfe()};
	}
}
bool FastCharacter::HasCard(const m2g::pb::CardType ct) const {
	for (const auto* card : _cards) {
		if (card && card->Type() == ct) {
			return true;
		}
	}
	return false;
}
bool FastCharacter::HasCard(const m2g::pb::CardCategory cc) const {
	for (const auto* card : _cards) {
		if (card && card->Category() == cc) {
			return true;
		}
	}
	return false;
}
size_t FastCharacter::CountCards(const m2g::pb::CardType ct) const {
	size_t count = 0;
	for (const auto* card : _cards) {
		if (card->Type() == ct) {
			++count;
		}
	}
	return count;
}
size_t FastCharacter::CountCards(const m2g::pb::CardCategory cc) const {
	size_t count = 0;
	for (const auto* card : _cards) {
		if (card->Category() == cc) {
			++count;
		}
	}
	return count;
}
std::optional<m2g::pb::CardType> FastCharacter::GetFirstCardType(const m2g::pb::CardCategory cc) const {
	if (const auto it = std::ranges::find_if(_cards, [=](const Card* card) { return card->Category() == cc; }); it != _cards.end()) {
		return (*it)->Type();
	}
	return std::nullopt;
}
void FastCharacter::AddCard(const m2g::pb::CardType ct) {
	_cards.emplace_back(&M2_GAME.GetCard(ct));
}
void FastCharacter::RemoveCard(const m2g::pb::CardType ct) {
	const auto* card = &M2_GAME.GetCard(ct);
	if (const auto it = std::ranges::find(_cards, card); it != _cards.end()) {
		_cards.erase(it);
	}
}
std::vector<m2g::pb::CardType> FastCharacter::GetCardTypes() const {
	std::vector<m2g::pb::CardType> retval;
	std::ranges::transform(_cards, std::back_inserter(retval), [](const Card* card) { return card->Type(); });
	return retval;
}
std::vector<m2g::pb::CardType> FastCharacter::GetCardTypes(const m2g::pb::CardCategory cc) const {
	std::vector<m2g::pb::CardType> retval;
	TransformIf(_cards.begin(), _cards.end(), std::back_inserter(retval),
		[=](const Card* card) { return card->Category() == cc; },
		[](const Card* card) { return card->Type(); });
	return retval;
}
const Card* FastCharacter::GetFirstCard(const m2g::pb::CardCategory cc) const {
	if (const auto it = std::ranges::find_if(_cards, [=](const Card* card) { return card->Category() == cc; }); it != _cards.end()) {
		return *it;
	}
	return nullptr;
}
