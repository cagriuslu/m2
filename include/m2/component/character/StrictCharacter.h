#pragma once
#include <m2/GameTypes.h>
#include <m2/Meta.h>
#include <m2g_CardType.pb.h>
#include <m2g_VariableType.pb.h>
#include <array>

namespace m2 {
	template <std::size_t N>
	using PossibleCardTypes = std::array<m2g::pb::CardType, N>;

	template <std::size_t N>
	using PossibleVariableTypes = std::array<m2g::pb::VariableType, N>;

	template <PossibleCardTypes possibleCardTypes, PossibleVariableTypes possibleVariableTypes>
	class StrictCharacter {
		// Verify that card types and variable types are unique
		static_assert(AreArrayElementsUnique(possibleCardTypes), "StrictCharacter supports only unique CardTypes");
		static_assert(AreArrayElementsUnique(possibleVariableTypes), "StrictCharacter supports only unique VariableTypes");

		static consteval int CardTypeIndex(m2g::pb::CardType it) {
			for (int i = 0; i < possibleCardTypes.size(); ++i) {
				if (possibleCardTypes[i] == it) { return i; }
			}
			return -1;
		}
		static consteval int VariableTypeIndex(m2g::pb::VariableType vt) {
			for (int i = 0; i < possibleVariableTypes.size(); ++i) {
				if (possibleVariableTypes[i] == vt) { return i; }
			}
			return -1;
		}

		std::array<bool, possibleCardTypes.size()> _cards;
		std::array<IFE, possibleVariableTypes.size()> _variables;

	public:

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
		[[nodiscard]] IFE GetVariable() const {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "This StrictCharacter specialization can't hold the given VariableType");
			return _variables[VariableTypeIndex(variableType)];
		}
		template <m2g::pb::VariableType variableType>
		IFE SetVariable(const IFE ife) {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "This StrictCharacter specialization can't hold the given VariableType");
			_variables[VariableTypeIndex(variableType)] = ife; return ife;
		}
	};
}
