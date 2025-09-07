#include <m2/Item.h>
#include <m2/protobuf/Detail.h>
#include <m2/Error.h>
#include <m2/Game.h>
#include <utility>

m2::Item::Item(pb::Item item) : _item(std::move(item)) {
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

std::pair<m2g::pb::ResourceType, float> m2::Item::GetCostByIndex(size_t i) const {
	const auto& cost = _item.costs((int) i);
	return std::make_pair(cost.type(), GetResourceAmount(cost));
}
float m2::Item::GetCost(m2g::pb::ResourceType type) const {
	return _costs[pb::enum_index(type)];
}
float m2::Item::TryGetCost(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetCost(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Item::HasCost(m2g::pb::ResourceType type) const {
	return GetCost(type) != 0.0f;
}
std::pair<m2g::pb::ResourceType, float> m2::Item::GetBenefitByIndex(size_t i) const {
	const auto& benefit = _item.benefits((int) i);
	return std::make_pair(benefit.type(), GetResourceAmount(benefit));
}
float m2::Item::GetBenefit(m2g::pb::ResourceType type) const {
	return _benefits[pb::enum_index(type)];
}
float m2::Item::TryGetBenefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetBenefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Item::HasBenefit(m2g::pb::ResourceType type) const {
	return GetBenefit(type) != 0.0f;
}
std::pair<m2g::pb::ResourceType, float> m2::Item::GetAcquireBenefitByIndex(size_t i) const {
	const auto& acquire_benefit = _item.acquire_benefits((int) i);
	return std::make_pair(acquire_benefit.type(), GetResourceAmount(acquire_benefit));
}
float m2::Item::GetAcquireBenefit(m2g::pb::ResourceType type) const {
	return _acquire_benefits[pb::enum_index(type)];
}
float m2::Item::TryGetAcquireBenefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetAcquireBenefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Item::HasAcquireBenefit(m2g::pb::ResourceType type) const {
	return GetAcquireBenefit(type) != 0.0f;
}
std::pair<m2g::pb::AttributeType, float> m2::Item::GetAttributeByIndex(size_t i) const {
	const auto& attr = _item.attributes((int) i);
	return std::make_pair(attr.type(), attr.amount());
}
float m2::Item::GetAttribute(m2g::pb::AttributeType type) const {
	return _attributes[pb::enum_index(type)];
}
float m2::Item::TryGetAttribute(m2g::pb::AttributeType type, float default_value) const {
	auto value = GetAttribute(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Item::HasAttribute(m2g::pb::AttributeType type) const {
	return GetAttribute(type) != 0.0f;
}

const m2::Item& m2::ToNamedItem(m2g::pb::ItemType item_type) {
	return M2_GAME.GetNamedItem(item_type);
}

std::function<float(const m2::Item&)> m2::ToAttributeValue(m2g::pb::AttributeType attribute_type) {
	return [attribute_type](const m2::Item& item) -> float {
		return item.GetAttribute(attribute_type);
	};
}

float m2::GetResourceAmount(const m2::pb::Resource& resource) {
	if (resource.has_amount()) {
		return resource.amount();
	}
	return 0.0f;
}
