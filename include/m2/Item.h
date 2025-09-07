#pragma once
#include "protobuf/Detail.h"
#include <Item.pb.h>
#include <vector>
#include <string>

namespace m2 {
	class Item {
		pb::Item _item;
		std::vector<float> _costs = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _benefits = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _acquire_benefits = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _attributes = std::vector<float>(pb::enum_value_count<m2g::pb::AttributeType>());

	public:
		Item() = default;
		explicit Item(pb::Item item);
		// Prevent accidental copying
		Item(const Item& other) = delete;
		Item& operator=(const Item& other) = delete;
		// Move allowed
		Item(Item&& other) = default;
		Item& operator=(Item&& other) = default;

		[[nodiscard]] m2g::pb::ItemType Type() const { return _item.type(); }
		[[nodiscard]] m2g::pb::ItemCategory Category() const { return _item.category(); }
		[[nodiscard]] pb::Usage Usage() const { return _item.usage(); }
		[[nodiscard]] bool UseOnAcquire() const { return _item.use_on_acquire(); }
		[[nodiscard]] size_t GetCostCount() const { return _item.costs_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetCostByIndex(size_t i) const;
		[[nodiscard]] float GetCost(m2g::pb::ResourceType) const;
		[[nodiscard]] float TryGetCost(m2g::pb::ResourceType, float default_value) const;
		[[nodiscard]] bool HasCost(m2g::pb::ResourceType) const;
		[[nodiscard]] size_t GetBenefitCount() const { return _item.benefits_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetBenefitByIndex(size_t i) const;
		[[nodiscard]] float GetBenefit(m2g::pb::ResourceType) const;
		[[nodiscard]] float TryGetBenefit(m2g::pb::ResourceType, float default_value) const;
		[[nodiscard]] bool HasBenefit(m2g::pb::ResourceType) const;
		[[nodiscard]] size_t GetAcquireBenefitCount() const { return _item.acquire_benefits_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetAcquireBenefitByIndex(size_t i) const;
		[[nodiscard]] float GetAcquireBenefit(m2g::pb::ResourceType) const;
		[[nodiscard]] float TryGetAcquireBenefit(m2g::pb::ResourceType, float default_value) const;
		[[nodiscard]] bool HasAcquireBenefit(m2g::pb::ResourceType) const;
		[[nodiscard]] size_t GetAttributeCount() const { return _item.attributes_size(); }
		[[nodiscard]] std::pair<m2g::pb::AttributeType, float> GetAttributeByIndex(size_t i) const;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType) const;
		[[nodiscard]] float TryGetAttribute(m2g::pb::AttributeType, float default_value) const;
		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType) const;
		[[nodiscard]] m2g::pb::SpriteType GameSprite() const { return _item.game_sprite(); }
		[[nodiscard]] m2g::pb::SpriteType UiSprite() const { return _item.ui_sprite(); }
		[[nodiscard]] const std::string& in_game_name() const { return _item.in_game_name(); }
	};

	// Transformers
	const Item& ToNamedItem(m2g::pb::ItemType item_type);
	std::function<float(const Item&)> ToAttributeValue(m2g::pb::AttributeType attribute_type);

	float GetResourceAmount(const pb::Resource& resource);
}
