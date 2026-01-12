#include <m2/component/Character.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <algorithm>
#include <m2/protobuf/Detail.h>
#include <cstdlib>

static_assert(std::forward_iterator<m2::Pool<m2::CharacterVariant>::Iterator>);

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

namespace {
	void tiny_character_iterator_incrementor(m2::Character::Iterator& it) {
		it.Set(nullptr);
	}
}

m2::CompactCharacter::CompactCharacter(uint64_t object_id) : Character(object_id) {}
int32_t m2::CompactCharacter::Hash(const int32_t initialValue) const {
	auto hash = initialValue;
	if (_item) {
		hash = HashI(_item->Type(), hash);
	}
	if (_resource.first) {
		throw M2_ERROR("CompactCharacter doesn't support hashing of resources");
	}
	if (_attribute.first) {
		throw M2_ERROR("CompactCharacter doesn't support hashing of attributes");
	}
	return hash;
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
	return _resource.first == resource_type && _resource.second != 0.0f;
}
float m2::CompactCharacter::GetResource(m2g::pb::ResourceType resource_type) const {
	return (_resource.first == resource_type) ? _resource.second : float{};
}
float m2::CompactCharacter::SetResource(m2g::pb::ResourceType resource_type, float amount) {
	_resource = std::make_pair(resource_type, amount);
	return _resource.second;
}
float m2::CompactCharacter::AddResource(m2g::pb::ResourceType resource_type, float amount) {
	if (_resource.first == m2g::pb::NO_RESOURCE) {
		SetResource(resource_type, 0.0f);
	}
	if (_resource.first == resource_type) {
		const auto currValue = _resource.second;
		const auto newValue = std::max(currValue + amount, 0.0f);
		_resource.second = newValue;
		return newValue;
	} else {
		throw M2_ERROR("Attempt to add/remove resource, but CompactCharacter doesn't carry that resource");
	}
}
float m2::CompactCharacter::RemoveResource(m2g::pb::ResourceType resource_type, float amount) {
	return AddResource(resource_type, -amount);
}
void m2::CompactCharacter::ClearResource(m2g::pb::ResourceType resource_type) {
	if (_resource.first == resource_type) {
		_resource.second = 0.0f;
	}
}
void m2::CompactCharacter::ClearResources() {
	_resource.second = 0.0f;
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
int32_t m2::FastCharacter::Hash(const int32_t initialValue) const {
	if constexpr (not GAME_IS_DETERMINISTIC) {
		// ReSharper disable once CppDFAUnreachableCode
		throw M2_ERROR("Game is not deterministic");
	}
	// ReSharper disable once CppDFAUnreachableCode
	auto hash = initialValue;
	for (const auto* item : _items) {
		if (item) {
			hash = HashI(item->Type(), hash);
		}
	}
	for (const auto& property : _properties) {
		if (property && property.IsInt()) {
			hash = HashI(property.GetInt(), hash);
		} else if (property && property.IsFE()) {
			hash = HashI(ToRawValue(property.GetFE()), hash);
		}
	}
	return hash;
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
	return _resources[ResourceTypeIndex(resource_type)] != 0.0f;
}
float m2::FastCharacter::GetResource(m2g::pb::ResourceType resource_type) const {
	return _resources[ResourceTypeIndex(resource_type)];
}
float m2::FastCharacter::SetResource(m2g::pb::ResourceType resource_type, float amount) {
	_resources[ResourceTypeIndex(resource_type)] = amount;
	return amount;
}
float m2::FastCharacter::AddResource(m2g::pb::ResourceType resource_type, float amount) {
	const auto currValue = _resources[ResourceTypeIndex(resource_type)];
	const auto newValue = std::max(currValue + amount, 0.0f);
	_resources[ResourceTypeIndex(resource_type)] = newValue;
	return newValue;
}
float m2::FastCharacter::RemoveResource(m2g::pb::ResourceType resource_type, float amount) {
	return AddResource(resource_type, -amount);
}
void m2::FastCharacter::ClearResource(m2g::pb::ResourceType resource_type) {
	_resources[ResourceTypeIndex(resource_type)] = 0.0f;
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

void m2::FastCharacter::AddPropertyMax(const m2g::pb::PropertyType pt, const FE& add, const FE& maxValue) {
	const auto currentValue = _properties[PropertyTypeIndex(pt)].GetFE();
	const auto addition = currentValue + add;
	const auto newValue = maxValue < addition ? maxValue : addition;
	_properties[PropertyTypeIndex(pt)] = IFE{newValue};
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
