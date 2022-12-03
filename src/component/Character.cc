#include <m2/component/Character.h>
#include <m2/Game.hh>
#include <algorithm>

m2::CharacterBase::CharacterBase(uint64_t object_id) : Component(object_id) {}
bool m2::CharacterBase::has_item(m2g::pb::ItemType item_type) const {
	return has_item(GAME.items[item_type]);
}
size_t m2::CharacterBase::count_item(m2g::pb::ItemType item_type) const {
	return count_item(GAME.items[item_type]);
}
void m2::CharacterBase::add_item(m2g::pb::ItemType item_type) {
	add_item(GAME.items[item_type]);
}
bool m2::CharacterBase::use_item(m2g::pb::ItemType item_type, float resource_multiplier) {
	return use_item(GAME.items[item_type], resource_multiplier);
}
bool m2::CharacterBase::use_item(const pb::Item& item, float resource_multiplier) {
	if (!has_item(item)) {
		return false;
	}
	// Check if costs can be paid
	if (item.costs_size() == 1) {
		if (0.0f < get_resource_amount(item.costs(0)) * resource_multiplier && this->get_resource(item.costs(0).type()) < get_resource_amount(item.costs(0)) * resource_multiplier) {
			return false;
		}
	} else if (1 < item.costs_size()) {
		// Merge costs
		std::map<m2g::pb::ResourceType, pb::Resource> merged_costs;
		for (const auto& cost_it : item.costs()) {
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
	for (const auto& cost_it : item.costs()) {
		this->remove_resource(cost_it.type(), get_resource_amount(cost_it) * resource_multiplier);
	}
	// Get the benefits
	for (const auto& benefit_it : item.benefits()) {
		this->add_resource(benefit_it.type(), get_resource_amount(benefit_it) * resource_multiplier);
	}
	if (item.usage() == pb::CONSUMABLE) {
		remove_item(item);
	}
	return true;
}
void m2::CharacterBase::remove_item(m2g::pb::ItemType item_type) {
	remove_item(GAME.items[item_type]);
}
void m2::CharacterBase::clear_item(m2g::pb::ItemType item_type) {
	clear_item(GAME.items[item_type]);
}
float m2::CharacterBase::get_resource_amount(const pb::Resource& resource) {
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

m2::TinyCharacter::TinyCharacter(uint64_t object_id) : CharacterBase(object_id) {}
void m2::TinyCharacter::automatic_update() {
	if (_item && _item->usage() == pb::AUTOMATIC) {
		use_item(*_item, GAME.deltaTime_s);
	}
}
bool m2::TinyCharacter::has_item(const pb::Item& item) const {
	return _item && _item->SerializeAsString() == item.SerializeAsString();
}
size_t m2::TinyCharacter::count_item(const pb::Item& item) const {
	return has_item(item) ? 1 : 0;
}
void m2::TinyCharacter::add_item(const pb::Item& item) {
	_item = item;
	if (item.use_on_acquire()) {
		use_item(item);
	}
}
void m2::TinyCharacter::remove_item(const pb::Item& item) {
	if (has_item(item)) {
		_item = {};
	}
}
void m2::TinyCharacter::clear_item(const pb::Item& item) {
	return remove_item(item);
}
bool m2::TinyCharacter::has_resource(m2g::pb::ResourceType resource_type) const {
	return _resource && _resource->first == resource_type && 0.0f < _resource->second;
}
float m2::TinyCharacter::get_resource(m2g::pb::ResourceType resource_type) const {
	if (_resource && _resource->first == resource_type) {
		return static_cast<float>(_resource->second);
	}
	return {};
}
float m2::TinyCharacter::add_resource(m2g::pb::ResourceType resource_type, float amount) {
	if (_resource && _resource->first == resource_type) {
		auto new_amount = _resource->second + amount;
		new_amount = (new_amount < 0.0f) ? 0.0f : new_amount;
		_resource->second = new_amount;
		return static_cast<float>(new_amount);
	} else if (0.0f < amount) {
		_resource = std::make_pair(resource_type, amount);
		return amount;
	} else {
		_resource = std::make_pair(resource_type, 0.0f);
		return 0.0f;
	}
}
float m2::TinyCharacter::remove_resource(m2g::pb::ResourceType resource_type, float amount) {
	return add_resource(resource_type, -amount);
}
void m2::TinyCharacter::clear_resource(m2g::pb::ResourceType resource_type) {
	if (_resource && _resource->first == resource_type) {
		_resource = {};
	}
}

m2::FullCharacter::FullCharacter(uint64_t object_id) : CharacterBase(object_id) {}
void m2::FullCharacter::automatic_update() {
	for (const auto& item : _items) {
		if (item.usage() == pb::AUTOMATIC) {
			use_item(item, GAME.deltaTime_s);
		}
	}
}
bool m2::FullCharacter::has_item(const pb::Item& item) const {
	auto serialized = item.SerializeAsString();
	return std::find_if(_items.cbegin(), _items.cend(), [serialized](const auto& held_item) {
		return serialized == held_item.SerializeAsString();
	}) != _items.end();
}
size_t m2::FullCharacter::count_item(const pb::Item& item) const {
	auto serialized = item.SerializeAsString();
	return std::count_if(_items.cbegin(), _items.cend(), [serialized](const auto& held_item) {
		return serialized == held_item.SerializeAsString();
	});
}
void m2::FullCharacter::add_item(const pb::Item& item) {
	_items.emplace_back(item);
	if (item.use_on_acquire()) {
		use_item(item);
	}
}
void m2::FullCharacter::remove_item(const pb::Item& item) {
	auto serialized = item.SerializeAsString();
	auto it = std::find_if(_items.cbegin(), _items.cend(), [serialized](const auto& held_item) {
		return serialized == held_item.SerializeAsString();
	});
	if (it != _items.end()) {
		_items.erase(it);
	}
}
void m2::FullCharacter::clear_item(const pb::Item& item) {
	auto serialized = item.SerializeAsString();
	_items.erase(std::remove_if(_items.begin(), _items.end(), [serialized](const auto& held_item) {
		return serialized == held_item.SerializeAsString();
	}), _items.end());
}
bool m2::FullCharacter::has_resource(m2g::pb::ResourceType resource_type) const {
	return 0.0f < _resources[resource_type];
}
float m2::FullCharacter::get_resource(m2g::pb::ResourceType resource_type) const {
	return _resources[resource_type];
}
float m2::FullCharacter::add_resource(m2g::pb::ResourceType resource_type, float amount) {
	auto new_amount = get_resource(resource_type) + amount;
	new_amount = (new_amount < 0.0f) ? 0.0f : new_amount;
	_resources[resource_type] = new_amount;
	return new_amount;
}
float m2::FullCharacter::remove_resource(m2g::pb::ResourceType resource_type, float amount) {
	return add_resource(resource_type, -amount);
}
void m2::FullCharacter::clear_resource(m2g::pb::ResourceType resource_type) {
	_resources[resource_type] = 0.0f;
}

m2::CharacterBase& m2::get_character_base(CharacterVariant& v) {
	return std::visit(m2::overloaded {
			[](TinyCharacter& v) -> CharacterBase& { return v; },
			[](FullCharacter& v) -> CharacterBase& { return v; },
	}, v);
}
