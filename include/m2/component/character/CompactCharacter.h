#pragma once
#include <m2/component/Character.h>

namespace m2 {
	class CompactCharacter final : public Character {
		std::optional<m2g::pb::CardType> _card;
		std::pair<m2g::pb::VariableType, IFE> _variable;

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
		void AddCard(m2g::pb::CardType) override;
		void RemoveCard(m2g::pb::CardType) override;

		[[nodiscard]] IFE GetVariable(m2g::pb::VariableType) const override;
		IFE SetVariable(m2g::pb::VariableType, IFE) override;
		void ClearVariable(m2g::pb::VariableType) override;
	};
}
