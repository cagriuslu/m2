#pragma once
#include <m2/component/Character.h>

namespace m2 {
	class CompactCharacter {
		ObjectId _ownerId;
		std::optional<m2g::pb::CardType> _card;
		std::pair<m2g::pb::VariableType, VariableValue> _variable;

	public:
		explicit CompactCharacter(uint64_t object_id);

		[[nodiscard]] ObjectId GetOwnerId() const { return _ownerId; }

		void OnUpdate(Stopwatch::Duration) {}
		void OnMessage(Interaction) {}
		template <reflect::IsVariantReflective T>
		void OnMessage(T&&) {}

		[[nodiscard]] int32_t Hash(int32_t initialValue) const;
		void Fill(pb::LockstepDebugStateReport::Character&) const;
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const;
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc);

		[[nodiscard]] bool HasCard(m2g::pb::CardType) const;
		[[nodiscard]] bool HasCard(m2g::pb::CardCategory) const;
		[[nodiscard]] int CountCards(m2g::pb::CardType) const;
		[[nodiscard]] int CountCards(m2g::pb::CardCategory) const;
		[[nodiscard]] m2g::pb::CardType GetFirstCardType(m2g::pb::CardCategory) const;
		expected<void> TryAddCard(m2g::pb::CardType);
		void UnsafeAddCard(m2g::pb::CardType);
		void RemoveCard(m2g::pb::CardType);

		[[nodiscard]] VariableValue GetVariable(m2g::pb::VariableType) const;
		bool TrySetVariable(m2g::pb::VariableType, VariableValue);
		void UnsafeSetVariable(m2g::pb::VariableType, VariableValue);
		void ClearVariable(m2g::pb::VariableType);
	};
	static_assert(CharacterImpl<CompactCharacter>);
}
