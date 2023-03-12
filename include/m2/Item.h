#ifndef M2_ITEM_H
#define M2_ITEM_H

#include "protobuf/Utils.h"
#include <Item.pb.h>
#include <vector>
#include <string>

namespace m2 {

	/// This class wraps m2::pb::Item
	/// It uses high memory, use only when fast resource/attribute lookup is needed.
	class Item {
		pb::Item _item;
		std::vector<float> _costs = std::vector<float>(proto::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _benefits = std::vector<float>(proto::enum_value_count<m2g::pb::ResourceType>());
		std::vector<float> _attributes = std::vector<float>(proto::enum_value_count<m2g::pb::AttributeType>());

	public:
		Item() = default;
		explicit Item(pb::Item item);

		const pb::Item& item() const;
		float get_cost(m2g::pb::ResourceType) const;
		float try_get_cost(m2g::pb::ResourceType, float default_value) const;
		bool has_cost(m2g::pb::ResourceType) const;
		float get_benefit(m2g::pb::ResourceType) const;
		float try_get_benefit(m2g::pb::ResourceType, float default_value) const;
		bool has_benefit(m2g::pb::ResourceType) const;
		float get_attribute(m2g::pb::AttributeType) const;
		float try_get_attribute(m2g::pb::AttributeType, float default_value) const;
		bool has_attribute(m2g::pb::AttributeType) const;
	};

	std::vector<Item> load_items(const std::string& items_path);
	float get_resource_amount(const pb::Resource& resource);
	Item example_damage_item(m2g::pb::ResourceType resource_type, float damage);
}

#endif //M2_ITEM_H
