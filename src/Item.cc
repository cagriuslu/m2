#include <m2/Item.h>
#include <m2/protobuf/Utils.h>
#include <m2/Exception.h>

m2::Item::Item(pb::Item item) : _item(std::move(item)) {
	for (const auto& cost : _item.costs()) {
		_costs[proto::enum_index(cost.type())] = get_resource_amount(cost);
	}
	for (const auto& benefit : _item.benefits()) {
		_benefits[proto::enum_index(benefit.type())] = get_resource_amount(benefit);
	}
	for (const auto& attribute : _item.attributes()) {
		_attributes[proto::enum_index(attribute.type())] = attribute.amount();
	}
}

const m2::pb::Item& m2::Item::item() const {
	return _item;
}
float m2::Item::get_cost(m2g::pb::ResourceType type) const {
	return _costs[proto::enum_index(type)];
}
float m2::Item::try_get_cost(m2g::pb::ResourceType type, float default_value) const {
	auto value = get_cost(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Item::has_cost(m2g::pb::ResourceType type) const {
	return get_cost(type) != 0.0f;
}
float m2::Item::get_benefit(m2g::pb::ResourceType type) const {
	return _benefits[proto::enum_index(type)];
}
float m2::Item::try_get_benefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = get_benefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Item::has_benefit(m2g::pb::ResourceType type) const {
	return get_benefit(type) != 0.0f;
}
float m2::Item::get_attribute(m2g::pb::AttributeType type) const {
	return _attributes[proto::enum_index(type)];
}
float m2::Item::try_get_attribute(m2g::pb::AttributeType type, float default_value) const {
	auto value = get_attribute(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Item::has_attribute(m2g::pb::AttributeType type) const {
	return get_attribute(type) != 0.0f;
}

std::vector<m2::Item> m2::load_items(const std::string &items_path) {
	auto items = proto::json_file_to_message<pb::Items>(items_path);
	if (!items) {
		throw M2ERROR(items.error());
	}

	std::vector<Item> items_vector(proto::enum_value_count<m2g::pb::ItemType>());
	std::vector<bool> is_loaded(proto::enum_value_count<m2g::pb::ItemType>());

	for (const auto& item : items->items()) {
		auto index = proto::enum_index(item.type());
		// Check if the item is already loaded
		if (is_loaded[index]) {
			throw M2ERROR("Item has duplicate definition: " + proto::enum_name(item.type()));
		}
		// Load item
		items_vector[index] = Item{item};
		is_loaded[index] = true;
	}

	// Check if every item is loaded
	for (int i = 0; i < proto::enum_value_count<m2g::pb::ItemType>(); ++i) {
		if (!is_loaded[i]) {
			throw M2ERROR("Item is not defined: " + proto::enum_name<m2g::pb::ItemType>(i));
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

m2::Item m2::example_damage_item(m2g::pb::ResourceType resource_type, float damage) {
	m2::pb::Item damage_item;

	damage_item.set_usage(m2::pb::CONSUMABLE);
	damage_item.set_use_on_acquire(true);
	auto* benefit = damage_item.add_benefits();
	benefit->set_type(resource_type);
	benefit->set_amount(-damage);

	return Item{damage_item};
}
