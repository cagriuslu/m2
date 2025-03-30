#include <m2/Item.h>
#include <m2/protobuf/Detail.h>
#include <m2/Error.h>
#include <m2/Game.h>
#include <utility>

m2::TinyItem::TinyItem(m2g::pb::ItemType type, m2g::pb::ItemCategory category, pb::Usage usage, bool use_on_acquire,
		std::pair<m2g::pb::ResourceType, float> cost, std::pair<m2g::pb::ResourceType, float> benefit,
		std::pair<m2g::pb::ResourceType, float> acquire_benefit, std::pair<m2g::pb::AttributeType, float> attribute,
		m2g::pb::SpriteType game_sprite, m2g::pb::SpriteType ui_sprite) : _type(type), _category(category), _usage(usage),
		_use_on_acquire(use_on_acquire), _cost(std::move(cost)), _benefit(std::move(benefit)), _acquire_benefit(std::move(acquire_benefit)),
		_attribute(std::move(attribute)), _game_sprite(game_sprite), _ui_sprite(ui_sprite) {}

std::pair<m2g::pb::ResourceType, float> m2::TinyItem::GetCostByIndex(size_t i) const {
	if (i == 0) {
		return _cost;
	} else {
		throw M2_ERROR("Out of bounds cost index");
	}
}
float m2::TinyItem::GetCost(m2g::pb::ResourceType resource_type) const {
	if (_cost.first == resource_type) {
		return _cost.second;
	}
	return 0.0f;
}
float m2::TinyItem::TryGetCost(m2g::pb::ResourceType resource_type, float default_value) const {
	if (_cost.first == resource_type) {
		return _cost.second;
	}
	return default_value;
}
bool m2::TinyItem::HasCost(m2g::pb::ResourceType resource_type) const {
	if (_cost.first == resource_type) {
		return _cost.second != 0.0f;
	}
	return false;
}
std::pair<m2g::pb::ResourceType, float> m2::TinyItem::GetBenefitByIndex(size_t i) const {
	if (i == 0) {
		return _benefit;
	} else {
		throw M2_ERROR("Out of bounds cost index");
	}
}
float m2::TinyItem::GetBenefit(m2g::pb::ResourceType resource_type) const {
	if (_benefit.first == resource_type) {
		return _benefit.second;
	}
	return 0.0f;
}
float m2::TinyItem::TryGetBenefit(m2g::pb::ResourceType resource_type, float default_value) const {
	if (_benefit.first == resource_type) {
		return _benefit.second;
	}
	return default_value;
}
bool m2::TinyItem::HasBenefit(m2g::pb::ResourceType resource_type) const {
	if (_benefit.first == resource_type) {
		return _benefit.second != 0.0f;
	}
	return false;
}
std::pair<m2g::pb::ResourceType, float> m2::TinyItem::GetAcquireBenefitByIndex(size_t i) const {
	if (i == 0) {
		return _acquire_benefit;
	} else {
		throw M2_ERROR("Out of bounds cost index");
	}
}
float m2::TinyItem::GetAcquireBenefit(m2g::pb::ResourceType resource_type) const {
	if (_acquire_benefit.first == resource_type) {
		return _acquire_benefit.second;
	}
	return 0.0f;
}
float m2::TinyItem::TryGetAcquireBenefit(m2g::pb::ResourceType resource_type, float default_value) const {
	if (_acquire_benefit.first == resource_type) {
		return _acquire_benefit.second;
	}
	return default_value;
}
bool m2::TinyItem::HasAcquireBenefit(m2g::pb::ResourceType resource_type) const {
	if (_acquire_benefit.first == resource_type) {
		return _acquire_benefit.second != 0.0f;
	}
	return false;
}
std::pair<m2g::pb::AttributeType, float> m2::TinyItem::GetAttributeByIndex(size_t i) const {
	if (i == 0) {
		return _attribute;
	} else {
		throw M2_ERROR("Out of bounds cost index");
	}
}
float m2::TinyItem::GetAttribute(m2g::pb::AttributeType attribute_type) const {
	if (_attribute.first == attribute_type) {
		return _attribute.second;
	}
	return 0.0f;
}
float m2::TinyItem::TryGetAttribute(m2g::pb::AttributeType attribute_type, float default_value) const {
	if (_attribute.first == attribute_type) {
		return _attribute.second;
	}
	return default_value;
}
bool m2::TinyItem::HasAttribute(m2g::pb::AttributeType attribute_type) const {
	if (_attribute.first == attribute_type) {
		return _attribute.second != 0.0f;
	}
	return false;
}

m2::NamedItem::NamedItem(pb::Item item) : _item(std::move(item)) {
	for (const auto& cost : _item.costs()) {
		_costs[pb::enum_index(cost.type())] = GetResourceAmount(cost);
	}
	for (const auto& benefit : _item.benefits()) {
		_benefits[pb::enum_index(benefit.type())] = GetResourceAmount(benefit);
	}
	for (const auto& acquire_benefit : _item.acquire_benefits()) {
		_acquire_benefits[pb::enum_index(acquire_benefit.type())] = GetResourceAmount(acquire_benefit);
	}
	for (const auto& attribute : _item.attributes()) {
		_attributes[pb::enum_index(attribute.type())] = attribute.amount();
	}
}

std::pair<m2g::pb::ResourceType, float> m2::NamedItem::GetCostByIndex(size_t i) const {
	const auto& cost = _item.costs((int) i);
	return std::make_pair(cost.type(), GetResourceAmount(cost));
}
float m2::NamedItem::GetCost(m2g::pb::ResourceType type) const {
	return _costs[pb::enum_index(type)];
}
float m2::NamedItem::TryGetCost(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetCost(type);
	return value != 0.0f ? value : default_value;
}
bool m2::NamedItem::HasCost(m2g::pb::ResourceType type) const {
	return GetCost(type) != 0.0f;
}
std::pair<m2g::pb::ResourceType, float> m2::NamedItem::GetBenefitByIndex(size_t i) const {
	const auto& benefit = _item.benefits((int) i);
	return std::make_pair(benefit.type(), GetResourceAmount(benefit));
}
float m2::NamedItem::GetBenefit(m2g::pb::ResourceType type) const {
	return _benefits[pb::enum_index(type)];
}
float m2::NamedItem::TryGetBenefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetBenefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::NamedItem::HasBenefit(m2g::pb::ResourceType type) const {
	return GetBenefit(type) != 0.0f;
}
std::pair<m2g::pb::ResourceType, float> m2::NamedItem::GetAcquireBenefitByIndex(size_t i) const {
	const auto& acquire_benefit = _item.acquire_benefits((int) i);
	return std::make_pair(acquire_benefit.type(), GetResourceAmount(acquire_benefit));
}
float m2::NamedItem::GetAcquireBenefit(m2g::pb::ResourceType type) const {
	return _acquire_benefits[pb::enum_index(type)];
}
float m2::NamedItem::TryGetAcquireBenefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetAcquireBenefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::NamedItem::HasAcquireBenefit(m2g::pb::ResourceType type) const {
	return GetAcquireBenefit(type) != 0.0f;
}
std::pair<m2g::pb::AttributeType, float> m2::NamedItem::GetAttributeByIndex(size_t i) const {
	const auto& attr = _item.attributes((int) i);
	return std::make_pair(attr.type(), attr.amount());
}
float m2::NamedItem::GetAttribute(m2g::pb::AttributeType type) const {
	return _attributes[pb::enum_index(type)];
}
float m2::NamedItem::TryGetAttribute(m2g::pb::AttributeType type, float default_value) const {
	auto value = GetAttribute(type);
	return value != 0.0f ? value : default_value;
}
bool m2::NamedItem::HasAttribute(m2g::pb::AttributeType type) const {
	return GetAttribute(type) != 0.0f;
}

const m2::NamedItem& m2::ToNamedItem(m2g::pb::ItemType item_type) {
	return M2_GAME.GetNamedItem(item_type);
}

std::function<float(const m2::NamedItem&)> m2::ToAttributeValue(m2g::pb::AttributeType attribute_type) {
	return [attribute_type](const m2::NamedItem& item) -> float {
		return item.GetAttribute(attribute_type);
	};
}

float m2::GetResourceAmount(const m2::pb::Resource& resource) {
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
