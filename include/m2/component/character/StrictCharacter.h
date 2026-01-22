#pragma once
#include <m2/Meta.h>
#include <m2/component/Character.h>
#include <m2/math/Hash.h>
#include <array>

namespace m2 {
	template <std::size_t N>
	using PossibleCardTypes = std::array<m2g::pb::CardType, N>;

	template <std::size_t N>
	using PossibleVariableTypes = std::array<m2g::pb::VariableType, N>;

	template <PossibleCardTypes possibleCardTypes, PossibleVariableTypes possibleVariableTypes = PossibleVariableTypes<0>{}>
	class StrictCharacter final : public Character {
		// Verify that card types and variable types are unique
		static_assert(AreArrayElementsUnique(possibleCardTypes), "StrictCharacter supports only unique CardTypes");
		static_assert(AreArrayElementsUnique(possibleVariableTypes), "StrictCharacter supports only unique VariableTypes");

		static constexpr int CardTypeIndex(const m2g::pb::CardType ct) {
			for (size_t i = 0; i < possibleCardTypes.size(); ++i) { if (possibleCardTypes[i] == ct) { return I(i); } }
			return -1;
		}
		static constexpr int VariableTypeIndex(const m2g::pb::VariableType vt) {
			for (size_t i = 0; i < possibleVariableTypes.size(); ++i) { if (possibleVariableTypes[i] == vt) { return I(i); } }
			return -1;
		}

		std::array<bool, possibleCardTypes.size()> _cards;
		std::array<IVFE, possibleVariableTypes.size()> _variables;

	public:
		using Character::Character;

		template <m2g::pb::CardType cardType>
		[[nodiscard]] bool HasCard() const {
			static_assert(DoesArrayContainElement(possibleCardTypes, cardType), "This StrictCharacter specialization can't hold the given CardType");
			return _cards[CardTypeIndex(cardType)];
		}
		template <m2g::pb::CardType cardType>
		void AddCardIfNotPresent() {
			static_assert(DoesArrayContainElement(possibleCardTypes, cardType), "This StrictCharacter specialization can't hold the given CardType");
			_cards[CardTypeIndex(cardType)] = true;
		}
		template <m2g::pb::CardType cardType>
		void RemoveCard() {
			static_assert(DoesArrayContainElement(possibleCardTypes, cardType), "This StrictCharacter specialization can't hold the given CardType");
			_cards[CardTypeIndex(cardType)] = false;
		}

		template <m2g::pb::VariableType variableType>
		[[nodiscard]] IVFE GetVariable() const {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "This StrictCharacter specialization can't hold the given VariableType");
			return _variables[VariableTypeIndex(variableType)];
		}
		template <m2g::pb::VariableType variableType>
		IVFE SetVariable(const IVFE ivfe) {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "This StrictCharacter specialization can't hold the given VariableType");
			_variables[VariableTypeIndex(variableType)] = ivfe; return ivfe;
		}

	protected:
		[[nodiscard]] int32_t Hash(int32_t hash) const override {
			if constexpr (not GAME_IS_DETERMINISTIC) {
				// ReSharper disable once CppDFAUnreachableCode
				throw M2_ERROR("Game is not deterministic");
			}
			// ReSharper disable once CppDFAUnreachableCode
			for (int i = 0; i < possibleCardTypes.size(); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (_cards[i]) {
					hash = HashI(possibleCardType, hash);
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
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const override {
			for (int i = 0; i < possibleCardTypes.size(); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (_cards[i]) {
					objDesc.add_cards(possibleCardType);
				}
			}
			for (int i = 0; i < possibleVariableTypes.size(); ++i) {
				const auto possibleVariableType = possibleVariableTypes[i];
				if (_variables[i]) {
					auto* var = objDesc.add_variables();
					var->set_type(possibleVariableType);
					var->mutable_ivfe()->CopyFrom(static_cast<pb::IVFE>(_variables[i]));
				}
			}
		}
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) override {
			_cards = std::array<bool, possibleCardTypes.size()>{};
			_variables = std::array<IVFE, possibleVariableTypes.size()>{};
			for (const auto& card : objDesc.cards()) {
				AddCard(static_cast<m2g::pb::CardType>(card));
			}
			for (const auto& variable : objDesc.variables()) {
				SetVariable(variable.type(), IVFE{variable.ivfe()});
			}
		}

		[[nodiscard]] bool HasCard(const m2g::pb::CardType ct) const override {
			if (const auto cardTypeIndex = CardTypeIndex(ct); cardTypeIndex == -1) {
				return false;
			} else {
				return _cards[cardTypeIndex];
			}
		}
		[[nodiscard]] bool HasCard(const m2g::pb::CardCategory cc) const override {
			for (int i = 0; i < possibleCardTypes.size(); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (const auto cat = ToCategoryOfCard(possibleCardType); cat == cc) {
					if (_cards[i]) { return true; }
				}
			}
			return false;
		}
		[[nodiscard]] size_t CountCards(const m2g::pb::CardType ct) const override {
			return HasCard(ct) ? 1 : 0;
		}
		[[nodiscard]] size_t CountCards(const m2g::pb::CardCategory cc) const override {
			size_t count = 0;
			for (int i = 0; i < possibleCardTypes.size(); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (const auto cat = ToCategoryOfCard(possibleCardType); cat == cc) {
					if (_cards[i]) { ++count; }
				}
			}
			return count;
		}
		[[nodiscard]] std::optional<m2g::pb::CardType> GetFirstCardType(const m2g::pb::CardCategory cc) const override {
			for (int i = 0; i < possibleCardTypes.size(); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (const auto cat = ToCategoryOfCard(possibleCardType); cat == cc) {
					if (_cards[i]) { return possibleCardType; }
				}
			}
			return std::nullopt;
		}
		void AddCard(const m2g::pb::CardType ct) override {
			if (const auto cardTypeIndex = CardTypeIndex(ct); cardTypeIndex == -1) {
				throw M2_ERROR("Strict character variant cannot hold the added card type");
			} else {
				if (_cards[cardTypeIndex]) {
					throw M2_ERROR("Strict character cannot hold more than one of each card type");
				}
				_cards[cardTypeIndex] = true;
			}
		}
		void RemoveCard(const m2g::pb::CardType ct) override {
			if (const auto cardTypeIndex = CardTypeIndex(ct); cardTypeIndex != -1) {
				_cards[cardTypeIndex] = false;
			}
		}

		[[nodiscard]] IVFE GetVariable(const m2g::pb::VariableType vt) const override {
			if (const auto variableTypeIndex = VariableTypeIndex(vt); variableTypeIndex == -1) {
				return {};
			} else {
				return _variables[variableTypeIndex];
			}
		}
		IVFE SetVariable(const m2g::pb::VariableType vt, IVFE ivfe) override {
			if (const auto variableTypeIndex = VariableTypeIndex(vt); variableTypeIndex == -1) {
				throw M2_ERROR("Strict character variant cannot hold the stored variable type");
			} else {
				_variables[variableTypeIndex] = ivfe; return ivfe;
			}
		}
		void ClearVariable(const m2g::pb::VariableType vt) override {
			if (const auto variableTypeIndex = VariableTypeIndex(vt); variableTypeIndex != -1) {
				_variables[variableTypeIndex] = {};
			}
		}
	};
}
