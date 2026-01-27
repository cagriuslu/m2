#pragma once
#include "../StrictCharacter.h"
#include <m2g_CardType.pb.h>
#include <array>

namespace m2 {
	template <std::size_t N>
	using PossibleCardTypes = std::array<m2g::pb::CardType, N>;

	template <PossibleCardTypes possibleCardTypes>
	class StrictCardSetCharacter {
		// Verify that card types are unique
		static_assert(AreArrayElementsUnique(possibleCardTypes), "StrictCardSetCharacter supports only unique CardTypes");

		static constexpr int CardTypeIndex(const m2g::pb::CardType ct) {
			for (int i = 0; i < I(possibleCardTypes.size()); ++i) { if (possibleCardTypes[i] == ct) { return i; } }
			return -1;
		}

		std::array<bool, possibleCardTypes.size()> _cards;

	public:
		[[nodiscard]] bool CanHoldCard(const m2g::pb::CardType ct) const {
			return CardTypeIndex(ct) != -1;
		}

		template <m2g::pb::CardType cardType>
		[[nodiscard]] bool HasCard() const {
			static_assert(DoesArrayContainElement(possibleCardTypes, cardType), "This StrictCardSetCharacter specialization can't hold the given CardType");
			return _cards[CardTypeIndex(cardType)];
		}
		template <m2g::pb::CardType cardType>
		void AddCardIfNotPresent() {
			static_assert(DoesArrayContainElement(possibleCardTypes, cardType), "This StrictCardSetCharacter specialization can't hold the given CardType");
			_cards[CardTypeIndex(cardType)] = true;
		}
		template <m2g::pb::CardType cardType>
		void RemoveCard() {
			static_assert(DoesArrayContainElement(possibleCardTypes, cardType), "This StrictCardSetCharacter specialization can't hold the given CardType");
			_cards[CardTypeIndex(cardType)] = false;
		}

		[[nodiscard]] bool HasCard(const m2g::pb::CardType ct) const {
			if (const auto cardTypeIndex = CardTypeIndex(ct); cardTypeIndex == -1) {
				return false;
			} else {
				return _cards[cardTypeIndex];
			}
		}
		[[nodiscard]] bool HasCard(const m2g::pb::CardCategory cc) const {
			for (int i = 0; i < I(possibleCardTypes.size()); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (const auto cat = ToCategoryOfCard(possibleCardType); cat == cc) {
					if (_cards[i]) { return true; }
				}
			}
			return false;
		}
		[[nodiscard]] size_t CountCards(const m2g::pb::CardType ct) const {
			return HasCard(ct) ? 1 : 0;
		}
		[[nodiscard]] size_t CountCards(const m2g::pb::CardCategory cc) const {
			size_t count = 0;
			for (int i = 0; i < I(possibleCardTypes.size()); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (const auto cat = ToCategoryOfCard(possibleCardType); cat == cc) {
					if (_cards[i]) { ++count; }
				}
			}
			return count;
		}
		[[nodiscard]] std::optional<m2g::pb::CardType> GetFirstCardType(const m2g::pb::CardCategory cc) const {
			for (int i = 0; i < I(possibleCardTypes.size()); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (const auto cat = ToCategoryOfCard(possibleCardType); cat == cc) {
					if (_cards[i]) { return possibleCardType; }
				}
			}
			return std::nullopt;
		}
		[[nodiscard]] bool TryAddCard(const m2g::pb::CardType ct) {
			if (const auto cardTypeIndex = CardTypeIndex(ct); cardTypeIndex == -1) {
				return false;
			} else {
				if (_cards[cardTypeIndex]) {
					return false;
				}
				_cards[cardTypeIndex] = true;
				return true;
			}
		}
		void UnsafeAddCard(const m2g::pb::CardType ct) {
			_cards[CardTypeIndex(ct)] = true;
		}
		void RemoveCard(const m2g::pb::CardType ct) {
			if (const auto cardTypeIndex = CardTypeIndex(ct); cardTypeIndex != -1) {
				_cards[cardTypeIndex] = false;
			}
		}

		[[nodiscard]] int32_t HashCards(int32_t hash) const {
			if constexpr (not GAME_IS_DETERMINISTIC) {
				// ReSharper disable once CppDFAUnreachableCode
				throw M2_ERROR("Game is not deterministic");
			}
			// ReSharper disable once CppDFAUnreachableCode
			for (int i = 0; i < I(possibleCardTypes.size()); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (_cards[i]) {
					hash = HashI(possibleCardType, hash);
				}
			}
			return hash;
		}
		void StoreCards(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const {
			for (int i = 0; i < I(possibleCardTypes.size()); ++i) {
				const auto possibleCardType = possibleCardTypes[i];
				if (_cards[i]) {
					objDesc.add_cards(possibleCardType);
				}
			}
		}
		void LoadCards(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
			_cards = std::array<bool, possibleCardTypes.size()>{};
			for (const auto& card : objDesc.cards()) {
				AddCard(static_cast<m2g::pb::CardType>(card));
			}
		}

		void AddCard(const m2g::pb::CardType ct) {
			if (const auto cardTypeIndex = CardTypeIndex(ct); cardTypeIndex == -1) {
				throw M2_ERROR("This StrictCardSetCharacter specialization cannot hold the added card type");
			} else {
				if (_cards[cardTypeIndex]) {
					throw M2_ERROR("Strict characters cannot hold more than one of each card type");
				}
				_cards[cardTypeIndex] = true;
			}
		}
	};
}

