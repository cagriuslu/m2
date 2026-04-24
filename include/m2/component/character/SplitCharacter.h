#pragma once
#include "CustomStateUtils.h"
#include <m2/Meta.h>
#include <m2/component/Character.h>
#include <m2/math/Hash.h>
#include <array>


namespace m2 {
	/// A character that can be customized with separate card and variable containers. Additionally, a CustomState can
	/// be held that is not mapped to any Card or Variable type. Since the Character containers are pre-allocated by
	/// the engine, CustomState must have a default constructor, which limits its usage. If the game requires holding
	/// custom state that can't be default initialized, it should prefer to use a PoolObjectImpl (or HeapObjectImpl)
	/// to precisely control when the object is constructed. If CustomStateUtilsImpl is implemented properly, the game
	/// engine will ensure proper synchronization and hash compression of the CustomState.
	template <typename CardContainer, typename VariableContainer, typename CustomState = std::monostate,
		typename CustomStateUtilsImpl = component::character::CustomStateUtils<CustomState>>
	class SplitCharacter final : public Character {
		CardContainer _cards;
		VariableContainer _variables;
		CustomState _customState{};

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
		[[nodiscard]] const VariableValue& GetVariable() const { return _variables.template GetVariable<variableType>(); }
		template <m2g::pb::VariableType variableType>
		[[nodiscard]] VariableValue& MutableVariable() { return _variables.template MutableVariable<variableType>(); }
		template <m2g::pb::VariableType variableType>
		void SetVariable(const VariableValue varVal) { _variables.template SetVariable<variableType>(varVal); }

		[[nodiscard]] bool HasCard(const m2g::pb::CardType ct) const override { return _cards.HasCard(ct); }
		[[nodiscard]] bool HasCard(const m2g::pb::CardCategory cc) const override { return _cards.HasCard(cc); }
		[[nodiscard]] size_t CountCards(const m2g::pb::CardType ct) const override { return _cards.CountCards(ct); }
		[[nodiscard]] size_t CountCards(const m2g::pb::CardCategory cc) const override { return _cards.CountCards(cc); }
		[[nodiscard]] std::optional<m2g::pb::CardType> GetFirstCardType(const m2g::pb::CardCategory cc) const override { return _cards.GetFirstCardType(cc); }
		[[nodiscard]] expected<void> TryAddCard(const m2g::pb::CardType ct) override { return _cards.TryAddCard(ct); }
		void UnsafeAddCard(const m2g::pb::CardType ct) override { _cards.UnsafeAddCard(ct); }
		void RemoveCard(const m2g::pb::CardType ct) override { _cards.RemoveCard(ct); }

		[[nodiscard]] const VariableValue& GetVariable(const m2g::pb::VariableType vt) const override { return _variables.GetVariable(vt); }
		[[nodiscard]] expected<void> TrySetVariable(const m2g::pb::VariableType vt, const VariableValue varVal) override { return _variables.TrySetVariable(vt, varVal); }
		void UnsafeSetVariable(const m2g::pb::VariableType vt, const VariableValue varVal) override { _variables.UnsafeSetVariable(vt, varVal); }
		void ClearVariable(const m2g::pb::VariableType vt) override { return _variables.ClearVariable(vt); }

		[[nodiscard]] const CustomState& GetCustomState() const { return _customState; }
		[[nodiscard]] CustomState& GetCustomState() { return _customState; }

	protected:
		[[nodiscard]] int32_t Hash(int32_t hash) const override {
			hash = _cards.HashCards(hash);
			hash = _variables.HashVariables(hash);
			return CustomStateUtilsImpl::Hash(_customState, hash);
		}
		void Fill(pb::LockstepDebugStateReport::Character& chrReport) const override {
			_cards.FillCards(*chrReport.mutable_cards());
			_variables.FillVariables(*chrReport.mutable_variables());
			CustomStateUtilsImpl::Fill(_customState, *chrReport.mutable_custom_state());
		}
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const override {
			_cards.StoreCards(objDesc);
			_variables.StoreVariables(objDesc);
			CustomStateUtilsImpl::Store(_customState, objDesc);
		}
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) override {
			_cards.LoadCards(objDesc);
			_variables.LoadVariables(objDesc);
			CustomStateUtilsImpl::Load(_customState, objDesc);
		}
	};
}
