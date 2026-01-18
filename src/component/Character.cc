#include <m2/component/Character.h>
#include <m2/Game.h>
#include <algorithm>
#include <m2/protobuf/Detail.h>

using namespace m2;

static_assert(std::forward_iterator<Pool<CharacterVariant>::Iterator>);

Character::Character(uint64_t object_id) : Component(object_id) {}
std::unique_ptr<const Proxy::InterCharacterMessage> Character::ExecuteInteraction(Character& initiator, std::unique_ptr<const Proxy::InterCharacterMessage>&& data) {
	if (this->onMessage) {
		return this->onMessage(*this, &initiator, data);
	}
	return {};
}
std::unique_ptr<const Proxy::InterCharacterMessage> Character::ExecuteInteraction(std::unique_ptr<const Proxy::InterCharacterMessage>&& data) {
	if (this->onMessage) {
		return this->onMessage(*this, nullptr, data);
	}
	return {};
}

int32_t Character::AddVariable(const m2g::pb::VariableType vt, const int32_t value, const std::optional<int32_t> maxValue) {
	if (GetVariable(vt).IsFE()) {
		throw M2_ERROR("Variable contains FE");
	}
	if (maxValue) {
		SetVariable(vt, std::min(GetVariable(vt).GetIntOrZero() + value, *maxValue));
	} else {
		SetVariable(vt, GetVariable(vt).GetIntOrZero() + value);
	}
	return GetVariable(vt).UnsafeGetInt();
}
FE Character::AddVariable(const m2g::pb::VariableType vt, const FE value, const std::optional<FE> maxValue) {
	if (GetVariable(vt).IsInt()) {
		throw M2_ERROR("Variable contains int32");
	}
	if (maxValue) {
		SetVariable(vt, std::min(GetVariable(vt).GetFEOrZero() + value, *maxValue));
	} else {
		SetVariable(vt, GetVariable(vt).GetFEOrZero() + value);
	}
	return GetVariable(vt).UnsafeGetFE();
}
int32_t Character::SubtractVariable(const m2g::pb::VariableType vt, const int32_t value, const std::optional<int32_t> minValue) {
	if (GetVariable(vt).IsFE()) {
		throw M2_ERROR("Variable contains FE");
	}
	if (minValue) {
		SetVariable(vt, std::max(GetVariable(vt).GetIntOrZero() - value, *minValue));
	} else {
		SetVariable(vt, GetVariable(vt).GetIntOrZero() - value);
	}
	return GetVariable(vt).UnsafeGetInt();
}
FE Character::SubtractVariable(const m2g::pb::VariableType vt, const FE value, const std::optional<FE> minValue) {
	if (GetVariable(vt).IsInt()) {
		throw M2_ERROR("Variable contains int32");
	}
	if (minValue) {
		SetVariable(vt, std::max(GetVariable(vt).GetFEOrZero() - value, *minValue));
	} else {
		SetVariable(vt, GetVariable(vt).GetFEOrZero() - value);
	}
	return GetVariable(vt).UnsafeGetFE();
}

CompactCharacter::CompactCharacter(uint64_t object_id) : Character(object_id) {}
int32_t CompactCharacter::Hash(const int32_t initialValue) const {
	auto hash = initialValue;
	if (_card) {
		hash = HashI(*_card, hash);
	}
	if (_variable.first && _variable.second) {
		if (_variable.second.IsInt()) {
			hash = HashI(_variable.second.UnsafeGetInt(), hash);
		} else if (_variable.second.IsFE()) {
			hash = HashI(ToRawValue(_variable.second.UnsafeGetFE()), hash);
		}
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
		var->mutable_ife()->CopyFrom(static_cast<pb::IFE>(_variable.second));
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
		_variable = std::make_pair(objDesc.variables(0).type(), IFE{objDesc.variables(0).ife()});
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
void CompactCharacter::AddCard(const m2g::pb::CardType ct) {
	if (_card) {
		throw M2_ERROR("CompactCharacter cannot hold more than one card");
	}
	_card = ct;
}
void CompactCharacter::RemoveCard(const m2g::pb::CardType ct) {
	if (_card == ct) {
		_card = std::nullopt;
	}
}

IFE CompactCharacter::GetVariable(const m2g::pb::VariableType v) const {
	if (_variable.first == v) {
		return _variable.second;
	}
	return {};
}
IFE CompactCharacter::SetVariable(const m2g::pb::VariableType v, const IFE ife) {
	if (_variable.first != m2g::pb::NO_VARIABLE && _variable.first != v) {
		throw M2_ERROR("CompactCharacter cannot hold more than one type of variables");
	}
	_variable = std::make_pair(v, ife);
	return ife;
}
void CompactCharacter::ClearVariable(const m2g::pb::VariableType v) {
	if (_variable.first == v) {
		_variable = {};
	}
}

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
			var->mutable_ife()->CopyFrom(static_cast<pb::IFE>(_variables[i]));
		}
	}
}
void FastCharacter::Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
	_cards.clear();
	_variables = std::vector<IFE>(pb::enum_value_count<m2g::pb::VariableType>());
	for (const auto& card : objDesc.cards()) {
		_cards.emplace_back(&M2_GAME.GetCard(static_cast<m2g::pb::CardType>(card)));
	}
	for (const auto& variable : objDesc.variables()) {
		_variables[pb::enum_index(variable.type())] = IFE{variable.ife()};
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

Character& m2::ToCharacterBase(CharacterVariant& v) {
	return std::visit([](auto& vv) -> Character& { return vv; }, v);
}
FastCharacter& m2::ToFastCharacter(CharacterVariant& v) {
	return std::visit(overloaded {
		[](FastCharacter& vv) -> FastCharacter& { return vv; },
		[](auto&) -> FastCharacter& { throw M2_ERROR("Unexpected character type"); }
	}, v);
}
const FastCharacter& m2::ToFastCharacter(const CharacterVariant& v) {
	return std::visit(overloaded {
		[](const FastCharacter& vv) -> const FastCharacter& { return vv; },
		[](auto&) -> const FastCharacter& { throw M2_ERROR("Unexpected character type"); }
	}, v);
}
