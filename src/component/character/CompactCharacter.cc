#include <m2/component/character/CompactCharacter.h>
#include <m2/Game.h>

using namespace m2;

CompactCharacter::CompactCharacter(uint64_t object_id) : Character(object_id) {}
int32_t CompactCharacter::Hash(const int32_t initialValue) const {
	auto hash = initialValue;
	if (_card) {
		hash = HashI(*_card, hash);
	}
	if (_variable.first && _variable.second) {
		hash = _variable.second.Hash(hash);
	}
	return hash;
}
void CompactCharacter::Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const {
	if (_card) {
		objDesc.add_cards(*_card);
	}
	if (_variable.first && _variable.second) {
		auto* var = objDesc.add_variables();
		var->set_type(_variable.first);
		var->mutable_var_val()->CopyFrom(static_cast<pb::VariableValue>(_variable.second));
	}
}
void CompactCharacter::Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
	if (1 < objDesc.cards_size()) {
		throw M2_ERROR("Attempted to load a compact character from turn-based object descriptor with multiple cards");
	}
	if (1 < objDesc.variables_size()) {
		throw M2_ERROR("Attempted to load a compact character from turn-based object descriptor with multiple variables");
	}
	_card = std::nullopt;
	_variable = {};
	if (objDesc.cards_size()) {
		_card = objDesc.cards(0);
	}
	if (objDesc.variables_size()) {
		_variable = std::make_pair(objDesc.variables(0).type(), VariableValue{objDesc.variables(0).var_val()});
	}
}
bool CompactCharacter::HasCard(const m2g::pb::CardType ct) const {
	return _card == ct;
}
bool CompactCharacter::HasCard(const m2g::pb::CardCategory cc) const {
	return _card && M2_GAME.GetCard(*_card).Category() == cc;
}
size_t CompactCharacter::CountCards(const m2g::pb::CardType ct) const {
	return HasCard(ct) ? 1 : 0;
}
size_t CompactCharacter::CountCards(const m2g::pb::CardCategory cc) const {
	return HasCard(cc) ? 1 : 0;
}
std::optional<m2g::pb::CardType> CompactCharacter::GetFirstCardType(const m2g::pb::CardCategory cc) const {
	return HasCard(cc) ? _card : std::optional<m2g::pb::CardType>{};
}
expected<void> CompactCharacter::TryAddCard(const m2g::pb::CardType ct) {
	if (_card) {
		return make_unexpected("Character cannot hold more than one card");
	}
	_card = ct;
	return {};
}
void CompactCharacter::UnsafeAddCard(const m2g::pb::CardType ct) {
	if (_card) {
		throw M2_ERROR("Character cannot hold more than one card");
	}
	_card = ct;
}
void CompactCharacter::RemoveCard(const m2g::pb::CardType ct) {
	if (_card == ct) {
		_card = std::nullopt;
	}
}

VariableValue CompactCharacter::GetVariable(const m2g::pb::VariableType v) const {
	if (_variable.first == v) {
		return _variable.second;
	}
	return {};
}
expected<VariableValue> CompactCharacter::TrySetVariable(const m2g::pb::VariableType v, const VariableValue varVal) {
	if (_variable.first && _variable.first != v) {
		return make_unexpected("Character cannot hold more than one type of variables");
	}
	_variable = std::make_pair(v, varVal);
	return varVal;
}
VariableValue CompactCharacter::UnsafeSetVariable(const m2g::pb::VariableType v, const VariableValue varVal) {
	if (_variable.first && _variable.first != v) {
		throw M2_ERROR("Character cannot hold more than one type of variables");
	}
	_variable = std::make_pair(v, varVal);
	return varVal;
}
void CompactCharacter::ClearVariable(const m2g::pb::VariableType v) {
	if (_variable.first == v) {
		_variable = {};
	}
}
