#include <m2/component/Character.h>
#include <m2/Game.h>
#include <algorithm>
#include <m2/protobuf/Detail.h>

using namespace m2;

static_assert(std::forward_iterator<m2::Pool<m2::CharacterVariant>::Iterator>);

m2::Character::Character(uint64_t object_id) : Component(object_id) {}
std::unique_ptr<const Proxy::InterCharacterMessage> m2::Character::ExecuteInteraction(Character& initiator, std::unique_ptr<const Proxy::InterCharacterMessage>&& data) {
	if (this->onMessage) {
		return this->onMessage(*this, &initiator, data);
	}
	return {};
}
std::unique_ptr<const Proxy::InterCharacterMessage> m2::Character::ExecuteInteraction(std::unique_ptr<const Proxy::InterCharacterMessage>&& data) {
	if (this->onMessage) {
		return this->onMessage(*this, nullptr, data);
	}
	return {};
}
bool m2::Character::HasCard(m2g::pb::CardType card_type) const {
    for (auto it = BeginCards(); it != EndCards(); ++it) {
        if (it->Type() == card_type) {
            return true;
        }
    }
	return false;
}
bool m2::Character::HasCard(m2g::pb::CardCategory card_cat) const {
    for (auto it = BeginCards(); it != EndCards(); ++it) {
        if (it->Category() == card_cat) {
            return true;
        }
    }
	return false;
}
size_t m2::Character::CountCard(m2g::pb::CardType card_type) const {
    size_t count = 0;
    for (auto it = BeginCards(); it != EndCards(); ++it) {
        if (it->Type() == card_type) {
            ++count;
        }
    }
	return count;
}
size_t m2::Character::CountCard(m2g::pb::CardCategory card_cat) const {
    size_t count = 0;
    for (auto it = BeginCards(); it != EndCards(); ++it) {
        if (it->Category() == card_cat) {
            ++count;
        }
    }
    return count;
}
std::vector<m2g::pb::CardType> m2::Character::CardTypes() const {
	std::vector<m2g::pb::CardType> types;
	for (auto it = BeginCards(); it != EndCards(); ++it) {
		types.emplace_back(it->Type());
	}
	return types;
}
std::vector<m2g::pb::CardType> m2::Character::CardTypes(const m2g::pb::CardCategory card_cat) const {
	std::vector<m2g::pb::CardType> types;
	for (auto it = FindCards(card_cat); it != EndCards(); ++it) {
		types.emplace_back(it->Type());
	}
	return types;
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

namespace {
	void tiny_character_iterator_incrementor(m2::Character::Iterator& it) {
		it.Set(nullptr);
	}
}

m2::CompactCharacter::CompactCharacter(uint64_t object_id) : Character(object_id) {}
int32_t m2::CompactCharacter::Hash(const int32_t initialValue) const {
	auto hash = initialValue;
	if (_card) {
		hash = HashI(_card->Type(), hash);
	}
	return hash;
}
m2::Character::Iterator m2::CompactCharacter::FindCards(m2g::pb::CardType card_type) const {
	return {*this, tiny_character_iterator_incrementor, card_type, 0,
			_card && _card->Type() == card_type ? _card : nullptr};
}
m2::Character::Iterator m2::CompactCharacter::FindCards(m2g::pb::CardCategory cat) const {
	return {*this, tiny_character_iterator_incrementor, cat, 0,
			_card && _card->Category() == cat ? _card : nullptr};
}
m2::Character::Iterator m2::CompactCharacter::BeginCards() const {
	return {*this, tiny_character_iterator_incrementor, {}, 0, _card};
}
m2::Character::Iterator m2::CompactCharacter::EndCards() const {
	return {*this, tiny_character_iterator_incrementor, {}, 0, nullptr};
}
void m2::CompactCharacter::AddCard(const Card& card) {
	_card = &card;
}
void m2::CompactCharacter::RemoveCard(const Iterator& card) {
	if (card != EndCards()) {
		_card = {};
	}
}
void m2::CompactCharacter::ClearCards() {
	_card = {};
}

m2::IFE m2::CompactCharacter::GetVariable(const m2g::pb::VariableType v) const {
	if (_variable.first == v) {
		return _variable.second;
	}
	return {};
}
m2::IFE m2::CompactCharacter::SetVariable(const m2g::pb::VariableType v, const IFE ife) {
	if (_variable.first != m2g::pb::NO_VARIABLE && _variable.first != v) {
		throw M2_ERROR("CompactCharacter cannot hold more than one type of variables");
	}
	_variable = std::make_pair(v, ife);
	return ife;
}
void m2::CompactCharacter::ClearVariable(const m2g::pb::VariableType v) {
	if (_variable.first == v) {
		_variable = {};
	}
}
void m2::CompactCharacter::ClearVariables() {
	_variable = {};
}

void m2::FullCharacterIteratorIncrementor(m2::Character::Iterator& it) {
	const auto& character = dynamic_cast<const m2::FastCharacter&>(it.GetCharacter());
	auto curr_index = it.GetIndex();
	auto filter = it.GetFilter();
	if (std::holds_alternative<std::monostate>(filter)) {
		if (curr_index + 1 < character._cards.size()) {
			// Next card
			it.SetIndex(curr_index + 1);
			it.Set(character._cards[curr_index + 1]);
			return;
		}
	} else if (std::holds_alternative<m2g::pb::CardType>(filter)) {
		for (size_t i = curr_index + 1; i < character._cards.size(); ++i) {
			if (character._cards[i]->Type() == std::get<m2g::pb::CardType>(filter)) {
				// Found card
				it.SetIndex(i);
				it.Set(character._cards[i]);
				return;
			}
		}
	} else if (std::holds_alternative<m2g::pb::CardCategory>(filter)) {
		for (size_t i = curr_index + 1; i < character._cards.size(); ++i) {
			if (character._cards[i]->Category() == std::get<m2g::pb::CardCategory>(filter)) {
				// Found card
				it.SetIndex(i);
				it.Set(character._cards[i]);
				return;
			}
		}
	} else {
		throw M2_ERROR("Invalid iterator filter");
	}
	// Card not found
	it.Set(nullptr);
}

m2::FastCharacter::FastCharacter(uint64_t object_id) : Character(object_id) {}
int32_t m2::FastCharacter::Hash(const int32_t initialValue) const {
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
m2::Character::Iterator m2::FastCharacter::FindCards(m2g::pb::CardType card_type) const {
	for (size_t i = 0; i < _cards.size(); ++i) {
		const auto& card = _cards[i];
		if (card->Type() == card_type) {
			return {*this, FullCharacterIteratorIncrementor, card_type, i, card};
		}
	}
	return EndCards();
}
m2::Character::Iterator m2::FastCharacter::FindCards(m2g::pb::CardCategory cat) const {
	for (size_t i = 0; i < _cards.size(); ++i) {
		const auto& card = _cards[i];
		if (card->Category() == cat) {
			return {*this, FullCharacterIteratorIncrementor, cat, i, card};
		}
	}
	return EndCards();
}
m2::Character::Iterator m2::FastCharacter::BeginCards() const {
	if (!_cards.empty()) {
		return {*this, FullCharacterIteratorIncrementor, {}, 0, _cards.front()};
	}
	return EndCards();
}
m2::Character::Iterator m2::FastCharacter::EndCards() const {
	return {*this, FullCharacterIteratorIncrementor, {}, 0, nullptr};
}
void m2::FastCharacter::AddCard(const Card& card) {
	_cards.emplace_back(&card);
}
void m2::FastCharacter::RemoveCard(const Iterator& card) {
	if (card != EndCards()) {
		auto it = _cards.cbegin();
		std::advance(it, card.GetIndex());
		_cards.erase(it);
	}
}
void m2::FastCharacter::ClearCards() {
	_cards.clear();
}

std::function<std::vector<m2g::pb::CardType>(m2::Character&)> m2::GenerateCardTypesFilter(m2g::pb::CardCategory card_category) {
	return [card_category](m2::Character& c) -> std::vector<m2g::pb::CardType> {
		return c.CardTypes(card_category);
	};
}
std::function<std::vector<m2g::pb::CardType>(m2::Character&)> m2::GenerateCardTypesFilter(std::initializer_list<m2g::pb::CardCategory> categoriesToFilter) {
	return [categoriesToFilter = std::move(categoriesToFilter)](const Character& c) -> std::vector<m2g::pb::CardType> {
		std::vector<m2g::pb::CardType> cardTypes;
		for (const auto& cat : categoriesToFilter) {
			auto _tmp = c.CardTypes(cat);
			cardTypes.insert(cardTypes.cend(), _tmp.begin(), _tmp.end());
		}
		return cardTypes;
	};
}

m2::Character& m2::ToCharacterBase(CharacterVariant& v) {
	return std::visit([](auto& vv) -> Character& { return vv; }, v);
}
m2::FastCharacter& m2::ToFastCharacter(CharacterVariant& v) {
	return std::visit(overloaded {
		[](FastCharacter& vv) -> FastCharacter& { return vv; },
		[](auto&) -> FastCharacter& { throw M2_ERROR("Unexpected character type"); }
	}, v);
}
const m2::FastCharacter& m2::ToFastCharacter(const CharacterVariant& v) {
	return std::visit(overloaded {
		[](const FastCharacter& vv) -> const FastCharacter& { return vv; },
		[](auto&) -> const FastCharacter& { throw M2_ERROR("Unexpected character type"); }
	}, v);
}
