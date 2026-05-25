#pragma once
#include <m2/component/Character.h>

namespace m2 {
	class FastCharacter {
		ObjectId _ownerId;
		std::vector<const Card*> _cards;
		std::vector<VariableValue> _variables = std::vector<VariableValue>(pb::enum_value_count<m2g::pb::VariableType>());

	public:
		explicit FastCharacter(uint64_t object_id);

		[[nodiscard]] ObjectId GetOwnerId() const { return _ownerId; }

		void OnUpdate(Stopwatch::Duration) {}
		void OnMessage(Interaction) {}

		[[nodiscard]] int32_t Hash(int32_t initialValue) const;
		void Fill(pb::LockstepDebugStateReport::Character&) const;
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const;
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc);

		[[nodiscard]] bool HasCard(m2g::pb::CardType) const;
		[[nodiscard]] bool HasCard(m2g::pb::CardCategory) const;
		[[nodiscard]] int CountCards(m2g::pb::CardType) const;
		[[nodiscard]] int CountCards(m2g::pb::CardCategory) const;
		[[nodiscard]] m2g::pb::CardType GetFirstCardType(m2g::pb::CardCategory) const;
		expected<void> TryAddCard(const m2g::pb::CardType ct) { AddCard(ct); return {}; }
		void UnsafeAddCard(const m2g::pb::CardType ct) { AddCard(ct); }
		void AddCard(m2g::pb::CardType);
		void RemoveCard(m2g::pb::CardType);

		[[nodiscard]] VariableValue GetVariable(const m2g::pb::VariableType v) const { return _variables[pb::enum_index(v)]; }
		bool TrySetVariable(const m2g::pb::VariableType vt, const VariableValue varVal) { SetVariable(vt, varVal); return true; }
		void UnsafeSetVariable(const m2g::pb::VariableType vt, const VariableValue varVal) { SetVariable(vt, varVal); }
		void SetVariable(const m2g::pb::VariableType v, VariableValue varVal) { _variables[pb::enum_index(v)] = varVal; }
		void ClearVariable(const m2g::pb::VariableType v) { _variables[pb::enum_index(v)] = {}; }

		// Utilities

		[[nodiscard]] std::vector<m2g::pb::CardType> GetCardTypes() const;
		[[nodiscard]] std::vector<m2g::pb::CardType> GetCardTypes(m2g::pb::CardCategory) const;
		[[nodiscard]] const Card* GetFirstCard(m2g::pb::CardCategory) const;
	};
	static_assert(CharacterImpl<FastCharacter>);
}
