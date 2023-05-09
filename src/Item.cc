#include <m2/Item.h>
#include <m2/protobuf/Detail.h>
#include <m2/Exception.h>

#include <utility>

m2::TinyItem::TinyItem(m2g::pb::ItemType type, m2g::pb::ItemCategory category, pb::Usage usage, bool use_on_acquire,
		std::pair<m2g::pb::ResourceType, float> cost, std::pair<m2g::pb::ResourceType, float> benefit,
		std::pair<m2g::pb::ResourceType, float> acquire_benefit, std::pair<m2g::pb::AttributeType, float> attribute,
		m2g::pb::SpriteType game_sprite, m2g::pb::SpriteType ui_sprite) : _type(type), _category(category), _usage(usage),
		_use_on_acquire(use_on_acquire), _cost(std::move(cost)), _benefit(std::move(benefit)), _acquire_benefit(std::move(acquire_benefit)),
		_attribute(std::move(attribute)), _game_sprite(game_sprite), _ui_sprite(ui_sprite) {}

std::pair<m2g::pb::ResourceType, float> m2::TinyItem::get_cost_by_index(size_t i) const {
	if (i == 0) {
		return _cost;
	} else {
		throw M2ERROR("Out of bounds cost index");
	}
}
float m2::TinyItem::get_cost(m2g::pb::ResourceType resource_type) const {
	if (_cost.first == resource_type) {
		return _cost.second;
	}
	return 0.0f;
}
float m2::TinyItem::try_get_cost(m2g::pb::ResourceType resource_type, float default_value) const {
	if (_cost.first == resource_type) {
		return _cost.second;
	}
	return default_value;
}
bool m2::TinyItem::has_cost(m2g::pb::ResourceType resource_type) const {
	if (_cost.first == resource_type) {
		return _cost.second != 0.0f;
	}
	return false;
}
std::pair<m2g::pb::ResourceType, float> m2::TinyItem::get_benefit_by_index(size_t i) const {
	if (i == 0) {
		return _benefit;
	} else {
		throw M2ERROR("Out of bounds cost index");
	}
}
float m2::TinyItem::get_benefit(m2g::pb::ResourceType resource_type) const {
	if (_benefit.first == resource_type) {
		return _benefit.second;
	}
	return 0.0f;
}
float m2::TinyItem::try_get_benefit(m2g::pb::ResourceType resource_type, float default_value) const {
	if (_benefit.first == resource_type) {
		return _benefit.second;
	}
	return default_value;
}
bool m2::TinyItem::has_benefit(m2g::pb::ResourceType resource_type) const {
	if (_benefit.first == resource_type) {
		return _benefit.second != 0.0f;
	}
	return false;
}
std::pair<m2g::pb::ResourceType, float> m2::TinyItem::get_acquire_benefit_by_index(size_t i) const {
	if (i == 0) {
		return _acquire_benefit;
	} else {
		throw M2ERROR("Out of bounds cost index");
	}
}
float m2::TinyItem::get_acquire_benefit(m2g::pb::ResourceType resource_type) const {
	if (_acquire_benefit.first == resource_type) {
		return _acquire_benefit.second;
	}
	return 0.0f;
}
float m2::TinyItem::try_get_acquire_benefit(m2g::pb::ResourceType resource_type, float default_value) const {
	if (_acquire_benefit.first == resource_type) {
		return _acquire_benefit.second;
	}
	return default_value;
}
bool m2::TinyItem::has_acquire_benefit(m2g::pb::ResourceType resource_type) const {
	if (_acquire_benefit.first == resource_type) {
		return _acquire_benefit.second != 0.0f;
	}
	return false;
}
std::pair<m2g::pb::AttributeType, float> m2::TinyItem::get_attribute_by_index(size_t i) const {
	if (i == 0) {
		return _attribute;
	} else {
		throw M2ERROR("Out of bounds cost index");
	}
}
float m2::TinyItem::get_attribute(m2g::pb::AttributeType attribute_type) const {
	if (_attribute.first == attribute_type) {
		return _attribute.second;
	}
	return 0.0f;
}
float m2::TinyItem::try_get_attribute(m2g::pb::AttributeType attribute_type, float default_value) const {
	if (_attribute.first == attribute_type) {
		return _attribute.second;
	}
	return default_value;
}
bool m2::TinyItem::has_attribute(m2g::pb::AttributeType attribute_type) const {
	if (_attribute.first == attribute_type) {
		return _attribute.second != 0.0f;
	}
	return false;
}

m2::FullItem::FullItem(pb::Item item) : _item(std::move(item)) {
	for (const auto& cost : _item.costs()) {
		_costs[protobuf::enum_index(cost.type())] = get_resource_amount(cost);
	}
	for (const auto& benefit : _item.benefits()) {
		_benefits[protobuf::enum_index(benefit.type())] = get_resource_amount(benefit);
	}
	for (const auto& acquire_benefit : _item.acquire_benefits()) {
		_acquire_benefits[protobuf::enum_index(acquire_benefit.type())] = get_resource_amount(acquire_benefit);
	}
	for (const auto& attribute : _item.attributes()) {
		_attributes[protobuf::enum_index(attribute.type())] = attribute.amount();
	}
}

std::pair<m2g::pb::ResourceType, float> m2::FullItem::get_cost_by_index(size_t i) const {
	const auto& cost = _item.costs((int) i);
	return std::make_pair(cost.type(), get_resource_amount(cost));
}
float m2::FullItem::get_cost(m2g::pb::ResourceType type) const {
	return _costs[protobuf::enum_index(type)];
}
float m2::FullItem::try_get_cost(m2g::pb::ResourceType type, float default_value) const {
	auto value = get_cost(type);
	return value != 0.0f ? value : default_value;
}
bool m2::FullItem::has_cost(m2g::pb::ResourceType type) const {
	return get_cost(type) != 0.0f;
}
std::pair<m2g::pb::ResourceType, float> m2::FullItem::get_benefit_by_index(size_t i) const {
	const auto& benefit = _item.benefits((int) i);
	return std::make_pair(benefit.type(), get_resource_amount(benefit));
}
float m2::FullItem::get_benefit(m2g::pb::ResourceType type) const {
	return _benefits[protobuf::enum_index(type)];
}
float m2::FullItem::try_get_benefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = get_benefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::FullItem::has_benefit(m2g::pb::ResourceType type) const {
	return get_benefit(type) != 0.0f;
}
std::pair<m2g::pb::ResourceType, float> m2::FullItem::get_acquire_benefit_by_index(size_t i) const {
	const auto& acquire_benefit = _item.acquire_benefits((int) i);
	return std::make_pair(acquire_benefit.type(), get_resource_amount(acquire_benefit));
}
float m2::FullItem::get_acquire_benefit(m2g::pb::ResourceType type) const {
	return _acquire_benefits[protobuf::enum_index(type)];
}
float m2::FullItem::try_get_acquire_benefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = get_acquire_benefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::FullItem::has_acquire_benefit(m2g::pb::ResourceType type) const {
	return get_acquire_benefit(type) != 0.0f;
}
std::pair<m2g::pb::AttributeType, float> m2::FullItem::get_attribute_by_index(size_t i) const {
	const auto& attr = _item.attributes((int) i);
	return std::make_pair(attr.type(), attr.amount());
}
float m2::FullItem::get_attribute(m2g::pb::AttributeType type) const {
	return _attributes[protobuf::enum_index(type)];
}
float m2::FullItem::try_get_attribute(m2g::pb::AttributeType type, float default_value) const {
	auto value = get_attribute(type);
	return value != 0.0f ? value : default_value;
}
bool m2::FullItem::has_attribute(m2g::pb::AttributeType type) const {
	return get_attribute(type) != 0.0f;
}

std::vector<m2::FullItem> m2::load_items(const std::string &items_path) {
	auto items = protobuf::json_file_to_message<pb::Items>(items_path);
	if (!items) {
		throw M2ERROR(items.error());
	}

	std::vector<FullItem> items_vector(protobuf::enum_value_count<m2g::pb::ItemType>());
	std::vector<bool> is_loaded(protobuf::enum_value_count<m2g::pb::ItemType>());

	for (const auto& item : items->items()) {
		auto index = protobuf::enum_index(item.type());
		// Check if the item is already loaded
		if (is_loaded[index]) {
			throw M2ERROR("Item has duplicate definition: " + protobuf::enum_name(item.type()));
		}
		// Load item
		items_vector[index] = FullItem{item};
		is_loaded[index] = true;
	}

	// Check if every item is loaded
	for (int i = 0; i < protobuf::enum_value_count<m2g::pb::ItemType>(); ++i) {
		if (!is_loaded[i]) {
			throw M2ERROR("Item is not defined: " + protobuf::enum_name<m2g::pb::ItemType>(i));
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

m2::SmartPointer<const m2::Item> m2::make_damage_item(m2g::pb::ResourceType resource_type, float damage) {
	return make_dynamic<const Item>(new TinyItem(m2g::pb::ItemType{}, m2g::pb::ItemCategory{}, pb::CONSUMABLE, true,
			std::pair<m2g::pb::ResourceType, float>{}, std::make_pair(resource_type, -damage),
			std::pair<m2g::pb::ResourceType, float>{}, std::pair<m2g::pb::AttributeType, float>{}, m2g::pb::SpriteType{},
			m2g::pb::SpriteType{}));
}
