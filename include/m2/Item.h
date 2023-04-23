#ifndef M2_ITEM_H
#define M2_ITEM_H

#include "protobuf/Detail.h"
#include "SmartPointer.h"
#include <Item.pb.h>
#include <vector>
#include <string>

namespace m2 {

	// Item exposes all fields in pb::Item explicitly because TinyItem does not hold a pb::Item
	class Item {
	public:
		virtual ~Item() = default;

		[[nodiscard]] virtual m2g::pb::ItemType type() const = 0;
		[[nodiscard]] virtual m2g::pb::ItemCategory category() const = 0;
		[[nodiscard]] virtual pb::Usage usage() const = 0;
		[[nodiscard]] virtual bool use_on_acquire() const = 0;
		[[nodiscard]] virtual size_t get_cost_count() const = 0;
		[[nodiscard]] virtual std::pair<m2g::pb::ResourceType, float> get_cost_by_index(size_t i) const = 0;
		[[nodiscard]] virtual float get_cost(m2g::pb::ResourceType) const = 0;
		[[nodiscard]] virtual float try_get_cost(m2g::pb::ResourceType, float default_value) const = 0;
		[[nodiscard]] virtual bool has_cost(m2g::pb::ResourceType) const = 0;
		[[nodiscard]] virtual size_t get_benefit_count() const = 0;
		[[nodiscard]] virtual std::pair<m2g::pb::ResourceType, float> get_benefit_by_index(size_t i) const = 0;
		[[nodiscard]] virtual float get_benefit(m2g::pb::ResourceType) const = 0;
		[[nodiscard]] virtual float try_get_benefit(m2g::pb::ResourceType, float default_value) const = 0;
		[[nodiscard]] virtual bool has_benefit(m2g::pb::ResourceType) const = 0;
		[[nodiscard]] virtual size_t get_attribute_count() const = 0;
		[[nodiscard]] virtual std::pair<m2g::pb::AttributeType, float> get_attribute_by_index(size_t i) const = 0;
		[[nodiscard]] virtual float get_attribute(m2g::pb::AttributeType) const = 0;
		[[nodiscard]] virtual float try_get_attribute(m2g::pb::AttributeType, float default_value) const = 0;
		[[nodiscard]] virtual bool has_attribute(m2g::pb::AttributeType) const = 0;
		[[nodiscard]] virtual m2g::pb::SpriteType game_sprite() const = 0;
		[[nodiscard]] virtual m2g::pb::SpriteType ui_sprite() const = 0;
	};

	/// TinyItem can represent an item with only one cost, benefit, and attribute
	class TinyItem final : public Item {
		m2g::pb::ItemType _type{};
		m2g::pb::ItemCategory _category{};
		pb::Usage _usage{};
		bool _use_on_acquire{};
		std::pair<m2g::pb::ResourceType, float> _cost{};
		std::pair<m2g::pb::ResourceType, float> _benefit{};
		std::pair<m2g::pb::AttributeType, float> _attribute{};
		m2g::pb::SpriteType _game_sprite{};
		m2g::pb::SpriteType _ui_sprite{};

	public:
		TinyItem() = default;
		TinyItem(m2g::pb::ItemType type, m2g::pb::ItemCategory category, pb::Usage usage, bool use_on_acquire,
				std::pair<m2g::pb::ResourceType, float> cost, std::pair<m2g::pb::ResourceType, float> benefit,
				std::pair<m2g::pb::AttributeType, float> attribute, m2g::pb::SpriteType game_sprite,
				m2g::pb::SpriteType ui_sprite);

		[[nodiscard]] inline m2g::pb::ItemType type() const override { return _type; }
		[[nodiscard]] inline m2g::pb::ItemCategory category() const override { return _category; }
		[[nodiscard]] inline pb::Usage usage() const override { return _usage; }
		[[nodiscard]] inline bool use_on_acquire() const override { return _use_on_acquire; }
		[[nodiscard]] inline size_t get_cost_count() const override { return _cost.second != 0.0f ? 1 : 0; }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> get_cost_by_index(size_t i) const override;
		[[nodiscard]] float get_cost(m2g::pb::ResourceType) const override;
		[[nodiscard]] float try_get_cost(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool has_cost(m2g::pb::ResourceType) const override;
		[[nodiscard]] inline size_t get_benefit_count() const override { return _benefit.second != 0.0f ? 1 : 0; }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> get_benefit_by_index(size_t i) const override;
		[[nodiscard]] float get_benefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] float try_get_benefit(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool has_benefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] inline size_t get_attribute_count() const override { return _attribute.second != 0.0f ? 1 : 0; }
		[[nodiscard]] std::pair<m2g::pb::AttributeType, float> get_attribute_by_index(size_t i) const override;
		[[nodiscard]] float get_attribute(m2g::pb::AttributeType) const override;
		[[nodiscard]] float try_get_attribute(m2g::pb::AttributeType, float default_value) const override;
		[[nodiscard]] bool has_attribute(m2g::pb::AttributeType) const override;
		[[nodiscard]] inline m2g::pb::SpriteType game_sprite() const override { return _game_sprite; }
		[[nodiscard]] inline m2g::pb::SpriteType ui_sprite() const override { return _ui_sprite; }
	};

	/// FullItem represents a full protobuf-backed item. It can have as many costs, benefits, and attributes as possible.
	/// This class uses high memory, and is slow to construct. It should only be used when fast resource/attribute lookup is necessary.
	class FullItem final : public Item {
		pb::Item _item;
		std::vector<float> _costs = std::vector<float>(protobuf::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _benefits = std::vector<float>(protobuf::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _attributes = std::vector<float>(protobuf::enum_value_count<m2g::pb::AttributeType>());

	public:
		FullItem() = default;
		explicit FullItem(pb::Item item);

		[[nodiscard]] inline m2g::pb::ItemType type() const override { return _item.type(); }
		[[nodiscard]] inline m2g::pb::ItemCategory category() const override { return _item.category(); }
		[[nodiscard]] inline pb::Usage usage() const override { return _item.usage(); }
		[[nodiscard]] inline bool use_on_acquire() const override { return _item.use_on_acquire(); }
		[[nodiscard]] inline size_t get_cost_count() const override { return _item.costs_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> get_cost_by_index(size_t i) const override;
		[[nodiscard]] float get_cost(m2g::pb::ResourceType) const override;
		[[nodiscard]] float try_get_cost(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool has_cost(m2g::pb::ResourceType) const override;
		[[nodiscard]] inline size_t get_benefit_count() const override { return _item.benefits_size(); }
		[[nodiscard]] std::pair<m2g::pb::ResourceType, float> get_benefit_by_index(size_t i) const override;
		[[nodiscard]] float get_benefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] float try_get_benefit(m2g::pb::ResourceType, float default_value) const override;
		[[nodiscard]] bool has_benefit(m2g::pb::ResourceType) const override;
		[[nodiscard]] inline size_t get_attribute_count() const override { return _item.attributes_size(); }
		[[nodiscard]] std::pair<m2g::pb::AttributeType, float> get_attribute_by_index(size_t i) const override;
		[[nodiscard]] float get_attribute(m2g::pb::AttributeType) const override;
		[[nodiscard]] float try_get_attribute(m2g::pb::AttributeType, float default_value) const override;
		[[nodiscard]] bool has_attribute(m2g::pb::AttributeType) const override;
		[[nodiscard]] inline m2g::pb::SpriteType game_sprite() const override { return _item.game_sprite(); }
		[[nodiscard]] inline m2g::pb::SpriteType ui_sprite() const override { return _item.ui_sprite(); }
	};

	std::vector<FullItem> load_items(const std::string& items_path);
	float get_resource_amount(const pb::Resource& resource);
	SmartPointer<const Item> make_damage_item(m2g::pb::ResourceType resource_type, float damage);
}

#endif //M2_ITEM_H
