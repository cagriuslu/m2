#pragma once
#include <m2/Meta.h>
#include <m2/component/Character.h>
#include <m2/math/Hash.h>
#include <array>

namespace m2 {
	template <typename StrictCardCharacter, typename StrictVariableCharacter>
	class StrictCharacter final : public Character {
		StrictCardCharacter _cards;
		StrictVariableCharacter _variables;

	public:
		using Character::Character;

		[[nodiscard]] bool CanHoldCard(const m2g::pb::CardType ct) const { return _cards.CanHoldCard(ct); }
		[[nodiscard]] bool CanHoldVariable(const m2g::pb::VariableType vt) const { return _variables.CanHoldVariable(vt); }

		template <m2g::pb::CardType cardType>
		[[nodiscard]] bool HasCard() const { return _cards.template HasCard<cardType>(); }
		template <m2g::pb::CardType cardType>
		void AddCardIfNotPresent() { return _cards.template AddCardIfNotPresent<cardType>(); }
		template <m2g::pb::CardType cardType>
		void RemoveCard() { return _cards.template RemoveCard<cardType>(); }

		template <m2g::pb::VariableType variableType>
		[[nodiscard]] IVFE GetVariable() const { return _variables.template GetVariable<variableType>(); }
		template <m2g::pb::VariableType variableType>
		IVFE SetVariable(const IVFE ivfe) { return _variables.template SetVariable<variableType>(ivfe); }

		[[nodiscard]] bool HasCard(const m2g::pb::CardType ct) const override { return _cards.HasCard(ct); }
		[[nodiscard]] bool HasCard(const m2g::pb::CardCategory cc) const override { return _cards.HasCard(cc); }
		[[nodiscard]] size_t CountCards(const m2g::pb::CardType ct) const override { return _cards.CountCards(ct); }
		[[nodiscard]] size_t CountCards(const m2g::pb::CardCategory cc) const override { return _cards.CountCards(cc); }
		[[nodiscard]] std::optional<m2g::pb::CardType> GetFirstCardType(const m2g::pb::CardCategory cc) const override { return _cards.GetFirstCardType(cc); }
		[[nodiscard]] expected<void> TryAddCard(const m2g::pb::CardType ct) override { return _cards.TryAddCard(ct); }
		void UnsafeAddCard(const m2g::pb::CardType ct) override { _cards.UnsafeAddCard(ct); }
		void RemoveCard(const m2g::pb::CardType ct) override { _cards.RemoveCard(ct); }

		[[nodiscard]] IVFE GetVariable(const m2g::pb::VariableType vt) const override { return _variables.GetVariable(vt); }
		[[nodiscard]] expected<IVFE> TrySetVariable(const m2g::pb::VariableType vt, const IVFE ivfe) override { return _variables.TrySetVariable(vt, ivfe); }
		IVFE UnsafeSetVariable(const m2g::pb::VariableType vt, const IVFE ivfe) override { return _variables.UnsafeSetVariable(vt, ivfe); }
		void ClearVariable(const m2g::pb::VariableType vt) override { return _variables.ClearVariable(vt); }

	protected:
		[[nodiscard]] int32_t Hash(int32_t hash) const override {
			hash = _cards.HashCards(hash);
			return _variables.HashVariables(hash);
		}
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const override {
			_cards.StoreCards(objDesc);
			_variables.StoreVariables(objDesc);
		}
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) override {
			_cards.LoadCards(objDesc);
			_variables.LoadVariables(objDesc);
		}
	};
}
