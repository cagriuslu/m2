#include <m2/component/Character.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <algorithm>
#include <m2/protobuf/Detail.h>
#include <cstdlib>

static_assert(std::forward_iterator<m2::Pool<m2::CharacterVariant>::Iterator>);

float m2::internal::ResourceAmount::SetMaxAmount(float max_amount) {
	if (max_amount < 0.0f) {
		throw M2_ERROR("Negative max resource");
	}
	_maxAmount = max_amount;
	return SetAmount(_amount);
}

m2::Character::Character(uint64_t object_id) : Component(object_id) {}
std::optional<m2g::pb::InteractionData> m2::Character::ExecuteInteraction(Character& initiator, const m2g::pb::InteractionData& data) {
	if (this->on_interaction) {
		return this->on_interaction(*this, &initiator, data);
	}
	return std::nullopt;
}
std::optional<m2g::pb::InteractionData> m2::Character::ExecuteInteraction(const m2g::pb::InteractionData& data) {
	if (this->on_interaction) {
		return this->on_interaction(*this, nullptr, data);
	}
	return std::nullopt;
}
bool m2::Character::HasItem(m2g::pb::ItemType item_type) const {
    for (auto it = BeginItems(); it != EndItems(); ++it) {
        if (it->Type() == item_type) {
            return true;
        }
    }
	return false;
}
bool m2::Character::HasItem(m2g::pb::ItemCategory item_cat) const {
    for (auto it = BeginItems(); it != EndItems(); ++it) {
        if (it->Category() == item_cat) {
            return true;
        }
    }
	return false;
}
size_t m2::Character::CountItem(m2g::pb::ItemType item_type) const {
    size_t count = 0;
    for (auto it = BeginItems(); it != EndItems(); ++it) {
        if (it->Type() == item_type) {
            ++count;
        }
    }
	return count;
}
size_t m2::Character::CountItem(m2g::pb::ItemCategory item_cat) const {
    size_t count = 0;
    for (auto it = BeginItems(); it != EndItems(); ++it) {
        if (it->Category() == item_cat) {
            ++count;
        }
    }
    return count;
}
std::vector<m2g::pb::ItemType> m2::Character::NamedItemTypes() const {
	std::vector<m2g::pb::ItemType> types;
	for (auto it = BeginItems(); it != EndItems(); ++it) {
		auto* item = it.Get();
		if (auto* named_item = dynamic_cast<const Item*>(item)) {
			types.emplace_back(named_item->Type());
		}
	}
	return types;
}
std::vector<m2g::pb::ItemType> m2::Character::NamedItemTypes(const m2g::pb::ItemCategory item_cat) const {
	std::vector<m2g::pb::ItemType> types;
	for (auto it = FindItems(item_cat); it != EndItems(); ++it) {
		auto* item = it.Get();
		if (auto* named_item = dynamic_cast<const Item*>(item)) {
			types.emplace_back(named_item->Type());
		}
	}
	return types;
}
bool m2::Character::UseItem(const Iterator& item_it, float resource_multiplier) {
	if (item_it == EndItems()) {
		return false;
	}
	if (item_it->Usage() == pb::STATIC) {
		LOG_WARN("Attempted to use STATIC item_it", item_it->Type());
		return false;
	}

	// Check if costs can be paid
	if (item_it->GetCostCount() == 1) {
		const auto cost = item_it->GetCostByIndex(0);
		const auto adjusted_cost_amount = cost.second * resource_multiplier;
		if (0.0f < adjusted_cost_amount && GetResource(cost.first) < adjusted_cost_amount) {
			return false;
		}
	} else if (1 < item_it->GetCostCount()) {
		// Merge costs
		auto resource_count = pb::enum_value_count<m2g::pb::ResourceType>();
		auto* merged_costs = (float*) alloca(resource_count * sizeof(float));
		memset(merged_costs, 0, resource_count * sizeof(float));
		for (size_t i = 0; i < item_it->GetCostCount(); ++i) {
			const auto cost = item_it->GetCostByIndex(i);
			merged_costs[pb::enum_index(cost.first)] += cost.second * resource_multiplier;
		}
		// Check if all costs are covered
		for (int i = 0; i < resource_count; ++i) {
			if (0.0f < merged_costs[i] && GetResource(pb::enum_value<m2g::pb::ResourceType>(i)) < merged_costs[i]) {
				return false;
			}
		}
	}

	// Pay the costs
	for (size_t i = 0; i < item_it->GetCostCount(); ++i) {
		const auto cost = item_it->GetCostByIndex(i);
		RemoveResource(cost.first, cost.second * resource_multiplier);
	}
	// Get the benefits
	for (size_t i = 0; i < item_it->GetBenefitCount(); ++i) {
		const auto benefit = item_it->GetBenefitByIndex(i);
		AddResource(benefit.first, benefit.second * resource_multiplier);
	}
	if (item_it->Usage() == pb::CONSUMABLE) {
		RemoveItem(item_it);
	}
	return true;
}

namespace {
	void tiny_character_iterator_incrementor(m2::Character::Iterator& it) {
		it.Set(nullptr);
	}
}

m2::CompactCharacter::CompactCharacter(uint64_t object_id) : Character(object_id) {}
void m2::CompactCharacter::AutomaticUpdate(const Stopwatch::Duration& delta) {
	if (_item && _item->Usage() == pb::AUTOMATIC) {
		UseItem(BeginItems(), ToDurationF(delta));
	}
}
m2::Character::Iterator m2::CompactCharacter::FindItems(m2g::pb::ItemType item_type) const {
	return {*this, tiny_character_iterator_incrementor, item_type, 0,
			_item && _item->Type() == item_type ? _item : nullptr};
}
m2::Character::Iterator m2::CompactCharacter::FindItems(m2g::pb::ItemCategory cat) const {
	return {*this, tiny_character_iterator_incrementor, cat, 0,
			_item && _item->Category() == cat ? _item : nullptr};
}
m2::Character::Iterator m2::CompactCharacter::BeginItems() const {
	return {*this, tiny_character_iterator_incrementor, {}, 0, _item};
}
m2::Character::Iterator m2::CompactCharacter::EndItems() const {
	return {*this, tiny_character_iterator_incrementor, {}, 0, nullptr};
}
void m2::CompactCharacter::AddNamedItem(const Item& item) {
	_item = &item;
	// Get acquire benefits
	for (size_t i = 0; i < _item->GetAcquireBenefitCount(); ++i) {
		const auto benefit = _item->GetAcquireBenefitByIndex(i);
		AddResource(benefit.first, benefit.second);
	}
	if (_item->UseOnAcquire()) {
		UseItem(BeginItems());
	}
}
void m2::CompactCharacter::AddNamedItemWithoutBenefits(const Item& item) {
	_item = &item;
}
void m2::CompactCharacter::RemoveItem(const Iterator& item) {
	if (item != EndItems()) {
		_item = {};
	}
}
void m2::CompactCharacter::ClearItems() {
	_item = {};
}
bool m2::CompactCharacter::HasResource(m2g::pb::ResourceType resource_type) const {
	return _resource.first == resource_type && _resource.second.HasAmount();
}
float m2::CompactCharacter::GetResource(m2g::pb::ResourceType resource_type) const {
	return (_resource.first == resource_type) ? _resource.second.Amount() : float{};
}
float m2::CompactCharacter::GetMaxResource(m2g::pb::ResourceType resource_type) const {
	return (_resource.first == resource_type) ? _resource.second.MaxAmount() : INFINITY;
}
void m2::CompactCharacter::SetMaxResource(m2g::pb::ResourceType resource_type, float max) {
	if (_resource.first == m2g::pb::NO_RESOURCE) {
		SetResource(resource_type, 0.0f);
	}

	if (_resource.first == resource_type) {
		_resource.second.SetMaxAmount(max);
	} else {
		LOG_WARN("Attempt to set max resource amount, but CompactCharacter doesn't carry that resource");
	}
}
float m2::CompactCharacter::SetResource(m2g::pb::ResourceType resource_type, float amount) {
	_resource = std::make_pair(resource_type, internal::ResourceAmount{amount});
	return _resource.second.Amount();
}
float m2::CompactCharacter::AddResource(m2g::pb::ResourceType resource_type, float amount) {
	if (_resource.first == m2g::pb::NO_RESOURCE) {
		SetResource(resource_type, 0.0f);
	}

	if (_resource.first == resource_type) {
		return _resource.second.AddAmount(amount);
	} else {
		LOG_WARN("Attempt to add/remove resource, but CompactCharacter doesn't carry that resource");
		return 0.0f;
	}
}
float m2::CompactCharacter::RemoveResource(m2g::pb::ResourceType resource_type, float amount) {
	return AddResource(resource_type, -amount);
}
void m2::CompactCharacter::ClearResource(m2g::pb::ResourceType resource_type) {
	if (_resource.first == resource_type) {
		_resource.second.ClearAmount();
	}
}
void m2::CompactCharacter::ClearResources() {
	_resource.second.ClearAmount();
}
bool m2::CompactCharacter::HasAttribute(m2g::pb::AttributeType attribute_type) const {
	return _attribute.first == attribute_type && _attribute.second != 0.0f;
}
float m2::CompactCharacter::GetAttribute(m2g::pb::AttributeType attribute_type) const {
	return (_attribute.first == attribute_type) ? _attribute.second : float{};
}
float m2::CompactCharacter::SetAttribute(m2g::pb::AttributeType attribute_type, float value) {
	_attribute = std::make_pair(attribute_type, value);
	return _attribute.second;
}
void m2::CompactCharacter::ClearAttribute(m2g::pb::AttributeType attribute_type) {
	if (_attribute.first == attribute_type) {
		_attribute.second = 0;
	}
}
void m2::CompactCharacter::ClearAttributes() {
	_attribute.second = 0;
}

void m2::FullCharacterIteratorIncrementor(m2::Character::Iterator& it) {
	const auto& character = dynamic_cast<const m2::FastCharacter&>(it.GetCharacter());
	auto curr_index = it.GetIndex();
	auto filter = it.GetFilter();
	if (std::holds_alternative<std::monostate>(filter)) {
		if (curr_index + 1 < character._items.size()) {
			// Next item
			it.SetIndex(curr_index + 1);
			it.Set(character._items[curr_index + 1]);
			return;
		}
	} else if (std::holds_alternative<m2g::pb::ItemType>(filter)) {
		for (size_t i = curr_index + 1; i < character._items.size(); ++i) {
			if (character._items[i]->Type() == std::get<m2g::pb::ItemType>(filter)) {
				// Found item
				it.SetIndex(i);
				it.Set(character._items[i]);
				return;
			}
		}
	} else if (std::holds_alternative<m2g::pb::ItemCategory>(filter)) {
		for (size_t i = curr_index + 1; i < character._items.size(); ++i) {
			if (character._items[i]->Category() == std::get<m2g::pb::ItemCategory>(filter)) {
				// Found item
				it.SetIndex(i);
				it.Set(character._items[i]);
				return;
			}
		}
	} else {
		throw M2_ERROR("Invalid iterator filter");
	}
	// Item not found
	it.Set(nullptr);
}

m2::FastCharacter::FastCharacter(uint64_t object_id) : Character(object_id) {}
void m2::FastCharacter::AutomaticUpdate(const Stopwatch::Duration& delta) {
	for (auto it = BeginItems(); it != EndItems(); ++it) {
		if (it->Usage() == pb::AUTOMATIC) {
			UseItem(it, ToDurationF(delta));
		}
	}
}
m2::Character::Iterator m2::FastCharacter::FindItems(m2g::pb::ItemType item_type) const {
	for (size_t i = 0; i < _items.size(); ++i) {
		const auto& item = _items[i];
		if (item->Type() == item_type) {
			return {*this, FullCharacterIteratorIncrementor, item_type, i, item};
		}
	}
	return EndItems();
}
m2::Character::Iterator m2::FastCharacter::FindItems(m2g::pb::ItemCategory cat) const {
	for (size_t i = 0; i < _items.size(); ++i) {
		const auto& item = _items[i];
		if (item->Category() == cat) {
			return {*this, FullCharacterIteratorIncrementor, cat, i, item};
		}
	}
	return EndItems();
}
m2::Character::Iterator m2::FastCharacter::BeginItems() const {
	if (!_items.empty()) {
		return {*this, FullCharacterIteratorIncrementor, {}, 0, _items.front()};
	}
	return EndItems();
}
m2::Character::Iterator m2::FastCharacter::EndItems() const {
	return {*this, FullCharacterIteratorIncrementor, {}, 0, nullptr};
}
void m2::FastCharacter::AddNamedItem(const Item& item) {
	_items.emplace_back(&item);
	// Get acquire benefits
	for (size_t i = 0; i < _items.back()->GetAcquireBenefitCount(); ++i) {
		const auto benefit = _items.back()->GetAcquireBenefitByIndex(i);
		AddResource(benefit.first, benefit.second);
	}
	if (_items.back()->UseOnAcquire()) {
		UseItem(Iterator{*this, FullCharacterIteratorIncrementor, {}, _items.size() - 1, _items.back()});
	}
}
void m2::FastCharacter::AddNamedItemWithoutBenefits(const Item& item) {
	_items.emplace_back(&item);
}
void m2::FastCharacter::RemoveItem(const Iterator& item) {
	if (item != EndItems()) {
		auto it = _items.cbegin();
		std::advance(it, item.GetIndex());
		_items.erase(it);
	}
}
void m2::FastCharacter::ClearItems() {
	_items.clear();
}
bool m2::FastCharacter::HasResource(m2g::pb::ResourceType resource_type) const {
	return _resources[ResourceTypeIndex(resource_type)].HasAmount();
}
float m2::FastCharacter::GetResource(m2g::pb::ResourceType resource_type) const {
	return _resources[ResourceTypeIndex(resource_type)].Amount();
}
float m2::FastCharacter::GetMaxResource(m2g::pb::ResourceType resource_type) const {
	return _resources[ResourceTypeIndex(resource_type)].MaxAmount();
}
void m2::FastCharacter::SetMaxResource(m2g::pb::ResourceType resource_type, float max) {
	_resources[ResourceTypeIndex(resource_type)].SetMaxAmount(max);
}
float m2::FastCharacter::SetResource(m2g::pb::ResourceType resource_type, float amount) {
	return _resources[ResourceTypeIndex(resource_type)].SetAmount(amount);
}
float m2::FastCharacter::AddResource(m2g::pb::ResourceType resource_type, float amount) {
	return _resources[ResourceTypeIndex(resource_type)].AddAmount(amount);
}
float m2::FastCharacter::RemoveResource(m2g::pb::ResourceType resource_type, float amount) {
	return _resources[ResourceTypeIndex(resource_type)].RemoveAmount(amount);
}
void m2::FastCharacter::ClearResource(m2g::pb::ResourceType resource_type) {
	_resources[ResourceTypeIndex(resource_type)].ClearAmount();
}
void m2::FastCharacter::ClearResources() {
	_resources.clear();
	_resources.resize(pb::enum_value_count<m2g::pb::ResourceType>());
}
bool m2::FastCharacter::HasAttribute(m2g::pb::AttributeType attribute_type) const {
	return _attributes[AttributeTypeIndex(attribute_type)] != 0.0f;
}
float m2::FastCharacter::GetAttribute(m2g::pb::AttributeType attribute_type) const {
	return _attributes[AttributeTypeIndex(attribute_type)];
}
float m2::FastCharacter::SetAttribute(m2g::pb::AttributeType attribute_type, float value) {
	_attributes[AttributeTypeIndex(attribute_type)] = value;
	return value;
}
void m2::FastCharacter::ClearAttribute(m2g::pb::AttributeType attribute_type) {
	_attributes[AttributeTypeIndex(attribute_type)] = 0.0f;
}
void m2::FastCharacter::ClearAttributes() {
	_attributes.clear();
	_attributes.resize(pb::enum_value_count<m2g::pb::AttributeType>());
}

int m2::FastCharacter::ResourceTypeIndex(m2g::pb::ResourceType resource_type) {
	return pb::enum_index<m2g::pb::ResourceType>(resource_type);
}
int m2::FastCharacter::AttributeTypeIndex(m2g::pb::AttributeType attribute_type) {
	return pb::enum_index<m2g::pb::AttributeType>(attribute_type);
}
int m2::FastCharacter::PropertyTypeIndex(m2g::pb::PropertyType pt) {
	return pb::enum_index<m2g::pb::PropertyType>(pt);
}

std::function<std::vector<m2g::pb::ItemType>(m2::Character&)> m2::GenerateNamedItemTypesFilter(m2g::pb::ItemCategory item_category) {
	return [item_category](m2::Character& c) -> std::vector<m2g::pb::ItemType> {
		return c.NamedItemTypes(item_category);
	};
}
std::function<std::vector<m2g::pb::ItemType>(m2::Character&)> m2::GenerateNamedItemTypesFilter(std::initializer_list<m2g::pb::ItemCategory> categoriesToFilter) {
	return [categoriesToFilter = std::move(categoriesToFilter)](const Character& c) -> std::vector<m2g::pb::ItemType> {
		std::vector<m2g::pb::ItemType> itemTypes;
		for (const auto& cat : categoriesToFilter) {
			auto _tmp = c.NamedItemTypes(cat);
			itemTypes.insert(itemTypes.cend(), _tmp.begin(), _tmp.end());
		}
		return itemTypes;
	};
}

m2::Character& m2::ToCharacterBase(CharacterVariant& v) {
	return std::visit([](auto& vv) -> Character& { return vv; }, v);
}
m2::FastCharacter& m2::ToFastCharacter(CharacterVariant& v) {
	return std::visit(overloaded {
		[](FastCharacter& vv) -> FastCharacter& { return vv; },
		[](auto&) -> FastCharacter& { throw M2_ERROR("Unexpected character type"); }
	}, v);
}
const m2::FastCharacter& m2::ToFastCharacter(const CharacterVariant& v) {
	return std::visit(overloaded {
		[](const FastCharacter& vv) -> const FastCharacter& { return vv; },
		[](auto&) -> const FastCharacter& { throw M2_ERROR("Unexpected character type"); }
	}, v);
}
