#pragma once
#include "protobuf/Detail.h"
#include <m2/GameTypes.h>
#include <Character.pb.h>
#include <vector>
#include <string>

namespace m2 {
	class Card {
		pb::Card _card;
		std::vector<float> _costs = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _benefits = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _acquire_benefits = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _attributes = std::vector<float>(pb::enum_value_count<m2g::pb::AttributeType>());
		std::vector<IFE> _constants = std::vector<IFE>(pb::enum_value_count<m2g::pb::ConstantType>());

	public:
		Card() = default;
		explicit Card(pb::Card card);
		// Prevent accidental copying
		Card(const Card& other) = delete;
		Card& operator=(const Card& other) = delete;
		// Move allowed
		Card(Card&& other) = default;
		Card& operator=(Card&& other) = default;

		[[nodiscard]] m2g::pb::CardType Type() const { return _card.type(); }
		[[nodiscard]] m2g::pb::CardCategory Category() const { return _card.category(); }
		[[nodiscard]] size_t GetCostCount() const { return _card.costs_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetCostByIndex(size_t i) const;
		[[nodiscard]] float GetCost(m2g::pb::ResourceType) const;
		[[nodiscard]] float TryGetCost(m2g::pb::ResourceType, float default_value) const;
		[[nodiscard]] bool HasCost(m2g::pb::ResourceType) const;
		[[nodiscard]] size_t GetBenefitCount() const { return _card.benefits_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetBenefitByIndex(size_t i) const;
		[[nodiscard]] float GetBenefit(m2g::pb::ResourceType) const;
		[[nodiscard]] float TryGetBenefit(m2g::pb::ResourceType, float default_value) const;
		[[nodiscard]] bool HasBenefit(m2g::pb::ResourceType) const;
		[[nodiscard]] size_t GetAcquireBenefitCount() const { return _card.acquire_benefits_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetAcquireBenefitByIndex(size_t i) const;
		[[nodiscard]] float GetAcquireBenefit(m2g::pb::ResourceType) const;
		[[nodiscard]] float TryGetAcquireBenefit(m2g::pb::ResourceType, float default_value) const;
		[[nodiscard]] bool HasAcquireBenefit(m2g::pb::ResourceType) const;
		[[nodiscard]] size_t GetAttributeCount() const { return _card.attributes_size(); }
		[[nodiscard]] std::pair<m2g::pb::AttributeType, float> GetAttributeByIndex(size_t i) const;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType) const;
		[[nodiscard]] float TryGetAttribute(m2g::pb::AttributeType, float default_value) const;
		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType) const;
		[[nodiscard]] IFE GetConstant(const m2g::pb::ConstantType c) const { return _constants[pb::enum_index(c)]; }
		[[nodiscard]] m2g::pb::SpriteType GameSprite() const { return _card.game_sprite(); }
		[[nodiscard]] m2g::pb::SpriteType UiSprite() const { return _card.ui_sprite(); }
		[[nodiscard]] const std::string& in_game_name() const { return _card.in_game_name(); }
	};

	// Transformers
	const Card& ToNamedCard(m2g::pb::CardType card_type);
	std::function<float(const Card&)> ToAttributeValue(m2g::pb::AttributeType attribute_type);

	float GetResourceAmount(const pb::Resource& resource);
}
