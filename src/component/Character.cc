#include <m2/component/Character.h>
#include <m2/Game.h>
#include <algorithm>

m2::Character::Character(uint64_t object_id) : Component(object_id) {}
void m2::Character::execute_interaction(Character& first_char, m2g::InteractionType cause, Character& second_char, m2g::InteractionType effect) {
	if (first_char.interact) {
		first_char.interact(first_char, second_char, cause);
	}
	if (second_char.interact) {
		second_char.interact(second_char, first_char, effect);
	}
}
bool m2::Character::has_item(m2g::pb::ItemType item_type) const {
    for (auto it = cbegin_items(); it != cend_items(); ++it) {
        if (it->item().type() == item_type) {
            return true;
        }
    }
	return false;
}
bool m2::Character::has_item(m2g::pb::ItemCategory item_cat) const {
    for (auto it = cbegin_items(); it != cend_items(); ++it) {
        if (it->item().category() == item_cat) {
            return true;
        }
    }
	return false;
}
size_t m2::Character::count_item(m2g::pb::ItemType item_type) const {
    size_t count = 0;
    for (auto it = cbegin_items(); it != cend_items(); ++it) {
        if (it->item().type() == item_type) {
            ++count;
        }
    }
	return count;
}
size_t m2::Character::count_item(m2g::pb::ItemCategory item_cat) const {
    size_t count = 0;
    for (auto it = cbegin_items(); it != cend_items(); ++it) {
        if (it->item().category() == item_cat) {
            ++count;
        }
    }
    return count;
}
bool m2::Character::use_item(const Iterator<Item>& item_it, float resource_multiplier) {
	if (item_it == end_items()) {
		return false;
	}
	if (item_it->item().usage() == pb::PASSIVE) {
		LOG_WARN("Attempted to use PASSIVE item_it", item_it->item().type());
		return false;
	}
	// Check if costs can be paid
	if (item_it->item().costs_size() == 1) {
		if (0.0f < get_resource_amount(item_it->item().costs(0)) * resource_multiplier && this->get_resource(item_it->item().costs(0).type()) < get_resource_amount(item_it->item().costs(0)) * resource_multiplier) {
			return false;
		}
	} else if (1 < item_it->item().costs_size()) {
		// Merge costs
		std::map<m2g::pb::ResourceType, pb::Resource> merged_costs;
		for (const auto& cost_it : item_it->item().costs()) {
			auto merged_costs_it = merged_costs.find(cost_it.type());
			if (merged_costs_it != merged_costs.end()) {
				auto new_cost = get_resource_amount(merged_costs_it->second) + get_resource_amount(cost_it);
				merged_costs_it->second.set_amount(new_cost);
			} else {
				merged_costs[cost_it.type()] = cost_it;
			}
		}
		bool enough = true;
		for (const auto& cost_it : merged_costs) {
			if (0.0f < get_resource_amount(cost_it.second) * resource_multiplier && this->get_resource(cost_it.second.type()) < get_resource_amount(cost_it.second) * resource_multiplier) {
				enough = false;
			}
		}
		if (!enough) {
			return false;
		}
	}
	// Pay the costs
	for (const auto& cost_it : item_it->item().costs()) {
		this->remove_resource(cost_it.type(), get_resource_amount(cost_it) * resource_multiplier);
	}
	// Get the benefits
	for (const auto& benefit_it : item_it->item().benefits()) {
		this->add_resource(benefit_it.type(), get_resource_amount(benefit_it) * resource_multiplier);
	}
	if (item_it->item().usage() == pb::CONSUMABLE) {
		remove_item(item_it);
	}
	return true;
}

float m2::internal::ResourceAmount::set_max_amount(float max_amount) {
	if (max_amount < 0.0f) {
		throw M2ERROR("Negative max resource");
	}
	_max_amount = max_amount;
	return set_amount(_amount);
}

m2::TinyCharacter::TinyCharacter(uint64_t object_id) : Character(object_id) {}
void m2::TinyCharacter::automatic_update() {
	if (_item && _item->item().usage() == pb::AUTOMATIC) {
		use_item(begin_items(), GAME.deltaTime_s);
	}
}
m2::Character::Iterator<m2::Item> m2::TinyCharacter::find_items(m2g::pb::ItemType item_type) {
	return {new TinyCharacterIteratorImpl<m2::Item>{}, item_type, _item && _item->item().type() == item_type ? &(*_item) : nullptr};
}
m2::Character::Iterator<m2::Item> m2::TinyCharacter::find_items(m2g::pb::ItemCategory cat) {
	return {new TinyCharacterIteratorImpl<m2::Item>{}, cat, _item && _item->item().category() == cat ? &(*_item) : nullptr};
}
m2::Character::Iterator<m2::Item> m2::TinyCharacter::begin_items() {
	return {new TinyCharacterIteratorImpl<m2::Item>{}, {}, _item ? &(*_item) : nullptr};
}
m2::Character::Iterator<m2::Item> m2::TinyCharacter::end_items() {
	return {nullptr, {}, nullptr};
}
m2::Character::Iterator<const m2::Item> m2::TinyCharacter::cbegin_items() const {
	return {new TinyCharacterIteratorImpl<const m2::Item>{}, {}, _item ? &(*_item) : nullptr};
}
m2::Character::Iterator<const m2::Item> m2::TinyCharacter::cend_items() const {
	return {nullptr, {}, nullptr};
}
void m2::TinyCharacter::add_item(const Item& item) {
	_item = item;
	if (item.item().use_on_acquire()) {
		use_item(begin_items());
	}
}
void m2::TinyCharacter::remove_item(const Iterator<Item>& item) {
	if (item != end_items()) {
		_item = {};
	}
}
bool m2::TinyCharacter::has_resource(m2g::pb::ResourceType resource_type) const {
	return _resource && _resource->first == resource_type && _resource->second.has_amount();
}
float m2::TinyCharacter::get_resource(m2g::pb::ResourceType resource_type) const {
	return (_resource && _resource->first == resource_type) ? _resource->second.amount() : float{};
}
float m2::TinyCharacter::get_max_resource(m2g::pb::ResourceType resource_type) const {
	return (_resource && _resource->first == resource_type) ? _resource->second.max_amount() : INFINITY;
}
void m2::TinyCharacter::set_max_resource(m2g::pb::ResourceType resource_type, float max) {
	if (_resource && _resource->first == resource_type) {
		_resource->second.set_max_amount(max);
	}
}
float m2::TinyCharacter::set_resource(m2g::pb::ResourceType resource_type, float amount) {
	if (_resource && _resource->first == resource_type) {
		return _resource->second.set_amount(amount);
	} else {
		_resource = std::make_pair(resource_type, internal::ResourceAmount{amount});
		return _resource->second.amount();
	}
}
float m2::TinyCharacter::add_resource(m2g::pb::ResourceType resource_type, float amount) {
	if (_resource && _resource->first == resource_type) {
		return _resource->second.add_amount(amount);
	} else {
		_resource = std::make_pair(resource_type, internal::ResourceAmount{amount});
		return _resource->second.amount();
	}
}
float m2::TinyCharacter::remove_resource(m2g::pb::ResourceType resource_type, float amount) {
	return add_resource(resource_type, -amount);
}
void m2::TinyCharacter::clear_resource(m2g::pb::ResourceType resource_type) {
	if (_resource && _resource->first == resource_type) {
		_resource->second.clear_amount();
	}
}

m2::FullCharacter::FullCharacter(uint64_t object_id) : Character(object_id) {}
void m2::FullCharacter::automatic_update() {
	for (auto it = begin_items(); it != end_items(); ++it) {
		if (it->item().usage() == pb::AUTOMATIC) {
			use_item(it, GAME.deltaTime_s);
		}
	}
}
m2::Character::Iterator<m2::Item> m2::FullCharacter::find_items(m2g::pb::ItemType item_type) {
	auto it = std::find_if(_items.begin(), _items.end(), [=](const Item& item) {
		return item.item().type() == item_type;
	});
	return {new FullCharacterIteratorImpl<m2::Item, FullCharacter>{this}, item_type, it == _items.end() ? nullptr : &(*it)};
}
m2::Character::Iterator<m2::Item> m2::FullCharacter::find_items(m2g::pb::ItemCategory cat) {
	auto it = std::find_if(_items.begin(), _items.end(), [=](const Item& item) {
		return item.item().category() == cat;
	});
	return {new FullCharacterIteratorImpl<m2::Item, FullCharacter>{this}, cat, it == _items.end() ? nullptr : &(*it)};
}
m2::Character::Iterator<m2::Item> m2::FullCharacter::begin_items() {
	return {new FullCharacterIteratorImpl<m2::Item, FullCharacter>{this}, {}, _items.empty() ? nullptr : _items.data()};
}
m2::Character::Iterator<m2::Item> m2::FullCharacter::end_items() {
	return {nullptr, {}, nullptr};
}
m2::Character::Iterator<const m2::Item> m2::FullCharacter::cbegin_items() const {
	return {new FullCharacterIteratorImpl<const m2::Item, const FullCharacter>{this}, {}, _items.empty() ? nullptr : _items.data()};
}
m2::Character::Iterator<const m2::Item> m2::FullCharacter::cend_items() const {
	return {nullptr, {}, nullptr};
}
void m2::FullCharacter::add_item(const Item& item) {
	_items.emplace_back(item);
	if (item.item().use_on_acquire()) {
		use_item(Iterator<Item>{nullptr, {}, &_items.back()});
	}
}
void m2::FullCharacter::remove_item(const Iterator<Item>& item) {
	if (item != end_items()) {
		auto start = _items.cbegin();
		std::advance(start, &(*item) - _items.data());
		_items.erase(start);
	}
}
bool m2::FullCharacter::has_resource(m2g::pb::ResourceType resource_type) const {
	return _resources[resource_type_index(resource_type)].has_amount();
}
float m2::FullCharacter::get_resource(m2g::pb::ResourceType resource_type) const {
	return _resources[resource_type_index(resource_type)].amount();
}
float m2::FullCharacter::get_max_resource(m2g::pb::ResourceType resource_type) const {
	return _resources[resource_type_index(resource_type)].max_amount();
}
void m2::FullCharacter::set_max_resource(m2g::pb::ResourceType resource_type, float max) {
	_resources[resource_type_index(resource_type)].set_max_amount(max);
}
float m2::FullCharacter::set_resource(m2g::pb::ResourceType resource_type, float amount) {
	return _resources[resource_type_index(resource_type)].set_amount(amount);
}
float m2::FullCharacter::add_resource(m2g::pb::ResourceType resource_type, float amount) {
	return _resources[resource_type_index(resource_type)].add_amount(amount);
}
float m2::FullCharacter::remove_resource(m2g::pb::ResourceType resource_type, float amount) {
	return _resources[resource_type_index(resource_type)].remove_amount(amount);
}
void m2::FullCharacter::clear_resource(m2g::pb::ResourceType resource_type) {
	_resources[resource_type_index(resource_type)].clear_amount();
}

int m2::FullCharacter::resource_type_index(m2g::pb::ResourceType resource_type) {
	return proto::enum_index<m2g::pb::ResourceType>(resource_type);
}

m2::Character& m2::get_character_base(CharacterVariant& v) {
	return std::visit(m2::overloaded {
			[](TinyCharacter& v) -> Character& { return v; },
			[](FullCharacter& v) -> Character& { return v; },
	}, v);
}
