#pragma once
#include <m2/component/Character.h>

namespace m2 {
	class FastCharacter final : public Character {
		std::vector<const Card*> _cards;
		std::vector<IVFE> _variables = std::vector<IVFE>(pb::enum_value_count<m2g::pb::VariableType>());

	public:
		FastCharacter() = default;
		explicit FastCharacter(uint64_t object_id);

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

		[[nodiscard]] IVFE GetVariable(const m2g::pb::VariableType v) const override { return _variables[pb::enum_index(v)]; }
		IVFE SetVariable(const m2g::pb::VariableType v, const IVFE ivfe) override { _variables[pb::enum_index(v)] = ivfe; return ivfe; }
		void ClearVariable(const m2g::pb::VariableType v) override { _variables[pb::enum_index(v)] = {}; }

		// Utilities

		[[nodiscard]] std::vector<m2g::pb::CardType> GetCardTypes() const;
		[[nodiscard]] std::vector<m2g::pb::CardType> GetCardTypes(m2g::pb::CardCategory) const;
		[[nodiscard]] const Card* GetFirstCard(m2g::pb::CardCategory) const;
	};
}
