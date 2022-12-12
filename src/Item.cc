#include <m2/Item.h>
#include <m2/protobuf/Utils.h>
#include <m2/Exception.h>

const google::protobuf::EnumDescriptor* const m2::Item::resource_type_desc = m2g::pb::ResourceType_descriptor();
const google::protobuf::EnumDescriptor* const m2::Item::attribute_type_desc = m2g::pb::AttributeType_descriptor();

m2::Item::Item(pb::Item item) : _item(std::move(item)) {
	for (const auto& cost : _item.costs()) {
		_costs[resource_type_desc->FindValueByNumber(cost.type())->index()] = get_resource_amount(cost);
	}
	for (const auto& benefit : _item.benefits()) {
		_benefits[resource_type_desc->FindValueByNumber(benefit.type())->index()] = get_resource_amount(benefit);
	}
	for (const auto& attribute : _item.attributes()) {
		_attributes[attribute_type_desc->FindValueByNumber(attribute.type())->index()] = attribute.amount();
	}
}
const m2::pb::Item* m2::Item::operator->() const {
	return &_item;
}
const m2::pb::Item& m2::Item::item() const {
	return _item;
}
float m2::Item::get_cost(m2g::pb::ResourceType type) const {
	return _costs[resource_type_desc->FindValueByNumber(type)->index()];
}
float m2::Item::get_benefit(m2g::pb::ResourceType type) const {
	return _benefits[resource_type_desc->FindValueByNumber(type)->index()];
}
float m2::Item::get_attribute(m2g::pb::AttributeType type) const {
	return _attributes[attribute_type_desc->FindValueByNumber(type)->index()];
}

std::vector<m2::Item> m2::load_items(const std::string &items_path) {
	auto items = proto::json_file_to_message<pb::Items>(items_path);
	if (!items) {
		throw M2ERROR(items.error());
	}

	std::vector<Item> items_vector(m2g::pb::ItemType_ARRAYSIZE);
	std::vector<bool> is_loaded(m2g::pb::ItemType_ARRAYSIZE);

	for (const auto& item : items->items()) {
		// Check if the item is already loaded
		if (is_loaded[item.type()]) {
			throw M2ERROR("Item has duplicate definition: " + std::to_string(item.type()));
		}
		// Load item
		items_vector[item.type()] = Item{item};
		is_loaded[item.type()] = true;
	}

	// Check if every item is loaded
	const auto* item_type_desc = m2g::pb::ItemType_descriptor();
	for (int e = 0; e < item_type_desc->value_count(); ++e) {
		int value = item_type_desc->value(e)->number();
		if (!is_loaded[value]) {
			throw M2ERROR("Item is not defined: " + std::to_string(value));
		}
	}

	return items_vector;
}

float m2::get_resource_amount(const m2::pb::Resource& resource) {
	if (resource.has_amount()) {
		return static_cast<float>(resource.amount());
	} else if (resource.has_p_inf() && resource.p_inf()) {
		return INFINITY;
	} else if (resource.has_n_inf() && resource.n_inf()) {
		return -INFINITY;
	} else {
		return 0.0f;
	}
}
