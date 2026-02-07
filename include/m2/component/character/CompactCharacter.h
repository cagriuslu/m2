#pragma once
#include <m2/component/Character.h>

namespace m2 {
	class CompactCharacter final : public Character {
		std::optional<m2g::pb::CardType> _card;
		std::pair<m2g::pb::VariableType, VariableValue> _variable;

	public:
		CompactCharacter() = default;
		explicit CompactCharacter(uint64_t object_id);

		[[nodiscard]] int32_t Hash(int32_t initialValue) const override;
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const override;
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) override;

		[[nodiscard]] bool HasCard(m2g::pb::CardType) const override;
		[[nodiscard]] bool HasCard(m2g::pb::CardCategory) const override;
		[[nodiscard]] size_t CountCards(m2g::pb::CardType) const override;
		[[nodiscard]] size_t CountCards(m2g::pb::CardCategory) const override;
		[[nodiscard]] std::optional<m2g::pb::CardType> GetFirstCardType(m2g::pb::CardCategory) const override;
		expected<void> TryAddCard(m2g::pb::CardType) override;
		void UnsafeAddCard(m2g::pb::CardType) override;
		void RemoveCard(m2g::pb::CardType) override;

		[[nodiscard]] const VariableValue& GetVariable(m2g::pb::VariableType) const override;
		expected<void> TrySetVariable(m2g::pb::VariableType, VariableValue) override;
		void UnsafeSetVariable(m2g::pb::VariableType, VariableValue) override;
		void ClearVariable(m2g::pb::VariableType) override;
	};
}
