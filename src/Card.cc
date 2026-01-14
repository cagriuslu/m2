#include <m2/Card.h>
#include <m2/protobuf/Detail.h>
#include <m2/Error.h>
#include <m2/Game.h>
#include <utility>

m2::Card::Card(pb::Card card) : _card(std::move(card)) {
	for (const auto& cost : _card.costs()) {
		_costs[pb::enum_index(cost.type())] = GetResourceAmount(cost);
	}
	for (const auto& benefit : _card.benefits()) {
		_benefits[pb::enum_index(benefit.type())] = GetResourceAmount(benefit);
	}
	for (const auto& acquire_benefit : _card.acquire_benefits()) {
		_acquire_benefits[pb::enum_index(acquire_benefit.type())] = GetResourceAmount(acquire_benefit);
	}
	for (const auto& attribute : _card.attributes()) {
		_attributes[pb::enum_index(attribute.type())] = attribute.amount();
	}
	for (const auto& attribute2 : _card.attributes2()) {
		_attributes2[pb::enum_index(attribute2.type())] = IFE{attribute2.ife()};
	}
	for (const auto& constant : _card.constants()) {
		_constants[pb::enum_index(constant.type())] = IFE{constant.ife()};
	}
}

std::pair<m2g::pb::ResourceType, float> m2::Card::GetCostByIndex(size_t i) const {
	const auto& cost = _card.costs(static_cast<int>(i));
	return std::make_pair(cost.type(), GetResourceAmount(cost));
}
float m2::Card::GetCost(m2g::pb::ResourceType type) const {
	return _costs[pb::enum_index(type)];
}
float m2::Card::TryGetCost(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetCost(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Card::HasCost(m2g::pb::ResourceType type) const {
	return GetCost(type) != 0.0f;
}
std::pair<m2g::pb::ResourceType, float> m2::Card::GetBenefitByIndex(size_t i) const {
	const auto& benefit = _card.benefits((int) i);
	return std::make_pair(benefit.type(), GetResourceAmount(benefit));
}
float m2::Card::GetBenefit(m2g::pb::ResourceType type) const {
	return _benefits[pb::enum_index(type)];
}
float m2::Card::TryGetBenefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetBenefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Card::HasBenefit(m2g::pb::ResourceType type) const {
	return GetBenefit(type) != 0.0f;
}
std::pair<m2g::pb::ResourceType, float> m2::Card::GetAcquireBenefitByIndex(size_t i) const {
	const auto& acquire_benefit = _card.acquire_benefits((int) i);
	return std::make_pair(acquire_benefit.type(), GetResourceAmount(acquire_benefit));
}
float m2::Card::GetAcquireBenefit(m2g::pb::ResourceType type) const {
	return _acquire_benefits[pb::enum_index(type)];
}
float m2::Card::TryGetAcquireBenefit(m2g::pb::ResourceType type, float default_value) const {
	auto value = GetAcquireBenefit(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Card::HasAcquireBenefit(m2g::pb::ResourceType type) const {
	return GetAcquireBenefit(type) != 0.0f;
}
std::pair<m2g::pb::AttributeType, float> m2::Card::GetAttributeByIndex(size_t i) const {
	const auto& attr = _card.attributes((int) i);
	return std::make_pair(attr.type(), attr.amount());
}
float m2::Card::GetAttribute(m2g::pb::AttributeType type) const {
	return _attributes[pb::enum_index(type)];
}
float m2::Card::TryGetAttribute(m2g::pb::AttributeType type, float default_value) const {
	auto value = GetAttribute(type);
	return value != 0.0f ? value : default_value;
}
bool m2::Card::HasAttribute(m2g::pb::AttributeType type) const {
	return GetAttribute(type) != 0.0f;
}
m2::IFE m2::Card::GetAttribute2(const m2g::pb::AttributeType type) const {
	return _attributes2[pb::enum_index(type)];
}
m2::IFE m2::Card::TryGetAttribute2(const m2g::pb::AttributeType type, const IFE& defaultValue) const {
	const auto value = GetAttribute2(type);
	return value ? value : defaultValue;
}
bool m2::Card::HasAttribute2(const m2g::pb::AttributeType type) const {
	return static_cast<bool>(GetAttribute2(type));
}

const m2::Card& m2::ToNamedCard(m2g::pb::CardType card_type) {
	return M2_GAME.GetNamedCard(card_type);
}

std::function<float(const m2::Card&)> m2::ToAttributeValue(m2g::pb::AttributeType attribute_type) {
	return [attribute_type](const m2::Card& card) -> float {
		return card.GetAttribute(attribute_type);
	};
}

float m2::GetResourceAmount(const m2::pb::Resource& resource) {
	if (resource.has_amount()) {
		return resource.amount();
	}
	return 0.0f;
}
