#pragma once
#include "protobuf/Detail.h"
#include <Item.pb.h>
#include <vector>
#include <string>

namespace m2 {
	// Item should not be stored, because it wouldn't be clear how it should be stored.
	// Use UnnamedItem and NamedItem instead. UnnamedItems are owned, NamedItems are referenced.
	class Item {
	public:
		virtual ~Item() = default;

		[[nodiscard]] virtual m2g::pb::ItemType Type() const = 0;
		[[nodiscard]] virtual m2g::pb::ItemCategory Category() const = 0;
		[[nodiscard]] virtual pb::Usage Usage() const = 0;
		[[nodiscard]] virtual bool UseOnAcquire() const = 0;

		[[nodiscard]] virtual size_t GetCostCount() const = 0;
		[[nodiscard]] virtual std::pair<m2g::pb::ResourceType, float> GetCostByIndex(size_t i) const = 0;
		[[nodiscard]] virtual float GetCost(m2g::pb::ResourceType) const = 0;
		[[nodiscard]] virtual float TryGetCost(m2g::pb::ResourceType, float default_value) const = 0;
		[[nodiscard]] virtual bool HasCost(m2g::pb::ResourceType) const = 0;

		[[nodiscard]] virtual size_t GetBenefitCount() const = 0;
		[[nodiscard]] virtual std::pair<m2g::pb::ResourceType, float> GetBenefitByIndex(size_t i) const = 0;
		[[nodiscard]] virtual float GetBenefit(m2g::pb::ResourceType) const = 0;
		[[nodiscard]] virtual float TryGetBenefit(m2g::pb::ResourceType, float default_value) const = 0;
		[[nodiscard]] virtual bool HasBenefit(m2g::pb::ResourceType) const = 0;

		[[nodiscard]] virtual size_t GetAcquireBenefitCount() const = 0;
		[[nodiscard]] virtual std::pair<m2g::pb::ResourceType, float> GetAcquireBenefitByIndex(size_t i) const = 0;
		[[nodiscard]] virtual float GetAcquireBenefit(m2g::pb::ResourceType) const = 0;
		[[nodiscard]] virtual float TryGetAcquireBenefit(m2g::pb::ResourceType, float default_value) const = 0;
		[[nodiscard]] virtual bool HasAcquireBenefit(m2g::pb::ResourceType) const = 0;

		[[nodiscard]] virtual size_t GetAttributeCount() const = 0;
		[[nodiscard]] virtual std::pair<m2g::pb::AttributeType, float> GetAttributeByIndex(size_t i) const = 0;
		[[nodiscard]] virtual float GetAttribute(m2g::pb::AttributeType) const = 0;
		[[nodiscard]] virtual float TryGetAttribute(m2g::pb::AttributeType, float default_value) const = 0;
		[[nodiscard]] virtual bool HasAttribute(m2g::pb::AttributeType) const = 0;

		[[nodiscard]] virtual m2g::pb::SpriteType GameSprite() const = 0;
		[[nodiscard]] virtual m2g::pb::SpriteType UiSprite() const = 0;
	};

	// Represents an item that doesn't have a protobuf correspondent.
	class UnnamedItem : public Item {};

	/// TinyItem can represent an item with only one cost, benefit, and attribute.
	class TinyItem final : public UnnamedItem {
		m2g::pb::ItemType _type{};
		m2g::pb::ItemCategory _category{};
		pb::Usage _usage{};
		bool _use_on_acquire{};
		std::pair<m2g::pb::ResourceType, float> _cost{};
		std::pair<m2g::pb::ResourceType, float> _benefit{};
		std::pair<m2g::pb::ResourceType, float> _acquire_benefit{};
		std::pair<m2g::pb::AttributeType, float> _attribute{};
		m2g::pb::SpriteType _game_sprite{};
		m2g::pb::SpriteType _ui_sprite{};

	public:
		TinyItem() = default;
		TinyItem(m2g::pb::ItemType type, m2g::pb::ItemCategory category, pb::Usage usage, bool use_on_acquire,
				std::pair<m2g::pb::ResourceType, float> cost, std::pair<m2g::pb::ResourceType, float> benefit,
				std::pair<m2g::pb::ResourceType, float> acquire_benefit, std::pair<m2g::pb::AttributeType, float> attribute,
				m2g::pb::SpriteType game_sprite, m2g::pb::SpriteType ui_sprite);

		[[nodiscard]] inline m2g::pb::ItemType Type() const override { return _type; }
		[[nodiscard]] inline m2g::pb::ItemCategory Category() const override { return _category; }
		[[nodiscard]] inline pb::Usage Usage() const override { return _usage; }
		[[nodiscard]] inline bool UseOnAcquire() const override { return _use_on_acquire; }
		[[nodiscard]] inline size_t GetCostCount() const override { return _cost.second != 0.0f ? 1 : 0; }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetCostByIndex(size_t i) const override;
		[[nodiscard]] float GetCost(m2g::pb::ResourceType) const override;
		[[nodiscard]] float TryGetCost(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool HasCost(m2g::pb::ResourceType) const override;
		[[nodiscard]] inline size_t GetBenefitCount() const override { return _benefit.second != 0.0f ? 1 : 0; }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetBenefitByIndex(size_t i) const override;
		[[nodiscard]] float GetBenefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] float TryGetBenefit(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool HasBenefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] size_t GetAcquireBenefitCount() const override { return _acquire_benefit.second != 0.0f ? 1 : 0; }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetAcquireBenefitByIndex(size_t i) const override;
		[[nodiscard]] float GetAcquireBenefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] float TryGetAcquireBenefit(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool HasAcquireBenefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] inline size_t GetAttributeCount() const override { return _attribute.second != 0.0f ? 1 : 0; }
		[[nodiscard]] std::pair<m2g::pb::AttributeType, float> GetAttributeByIndex(size_t i) const override;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType) const override;
		[[nodiscard]] float TryGetAttribute(m2g::pb::AttributeType, float default_value) const override;
		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType) const override;
		[[nodiscard]] inline m2g::pb::SpriteType GameSprite() const override { return _game_sprite; }
		[[nodiscard]] inline m2g::pb::SpriteType UiSprite() const override { return _ui_sprite; }
	};

	/// Represents a protobuf-backed item. It can have as many costs, benefits, and attributes as possible.
	/// This class uses high memory, and is slow to construct. It should only be used when fast resource/attribute lookup is necessary.
	class NamedItem final : public Item {
		pb::Item _item;
		std::vector<float> _costs = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _benefits = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _acquire_benefits = std::vector<float>(pb::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _attributes = std::vector<float>(pb::enum_value_count<m2g::pb::AttributeType>());

	public:
		NamedItem() = default;
		explicit NamedItem(pb::Item item);
		// Prevent accidental copying
		NamedItem(const NamedItem& other) = delete;
		NamedItem& operator=(const NamedItem& other) = delete;
		// Move allowed
		NamedItem(NamedItem&& other) = default;
		NamedItem& operator=(NamedItem&& other) = default;

		[[nodiscard]] inline m2g::pb::ItemType Type() const override { return _item.type(); }
		[[nodiscard]] inline m2g::pb::ItemCategory Category() const override { return _item.category(); }
		[[nodiscard]] inline pb::Usage Usage() const override { return _item.usage(); }
		[[nodiscard]] inline bool UseOnAcquire() const override { return _item.use_on_acquire(); }
		[[nodiscard]] inline size_t GetCostCount() const override { return _item.costs_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetCostByIndex(size_t i) const override;
		[[nodiscard]] float GetCost(m2g::pb::ResourceType) const override;
		[[nodiscard]] float TryGetCost(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool HasCost(m2g::pb::ResourceType) const override;
		[[nodiscard]] inline size_t GetBenefitCount() const override { return _item.benefits_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetBenefitByIndex(size_t i) const override;
		[[nodiscard]] float GetBenefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] float TryGetBenefit(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool HasBenefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] size_t GetAcquireBenefitCount() const override { return _item.acquire_benefits_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> GetAcquireBenefitByIndex(size_t i) const override;
		[[nodiscard]] float GetAcquireBenefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] float TryGetAcquireBenefit(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool HasAcquireBenefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] inline size_t GetAttributeCount() const override { return _item.attributes_size(); }
		[[nodiscard]] std::pair<m2g::pb::AttributeType, float> GetAttributeByIndex(size_t i) const override;
		[[nodiscard]] float GetAttribute(m2g::pb::AttributeType) const override;
		[[nodiscard]] float TryGetAttribute(m2g::pb::AttributeType, float default_value) const override;
		[[nodiscard]] bool HasAttribute(m2g::pb::AttributeType) const override;
		[[nodiscard]] inline m2g::pb::SpriteType GameSprite() const override { return _item.game_sprite(); }
		[[nodiscard]] inline m2g::pb::SpriteType UiSprite() const override { return _item.ui_sprite(); }
		[[nodiscard]] const std::string& in_game_name() const { return _item.in_game_name(); }
	};

	// Transformers
	const NamedItem& ToNamedItem(m2g::pb::ItemType item_type);
	std::function<float(const NamedItem&)> ToAttributeValue(m2g::pb::AttributeType attribute_type);

	float GetResourceAmount(const pb::Resource& resource);
}
