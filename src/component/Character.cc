#include <m2/component/Character.h>
#include <m2/Game.h>
#include <algorithm>
#include <m2/protobuf/Detail.h>
#include <cstdlib>

float m2::internal::ResourceAmount::set_max_amount(float max_amount) {
	if (max_amount < 0.0f) {
		throw M2ERROR("Negative max resource");
	}
	_max_amount = max_amount;
	return set_amount(_amount);
}

m2::Character::Character(uint64_t object_id) : Component(object_id) {}
void m2::Character::execute_interaction(Character& first_char, Character& second_char, m2g::pb::InteractionType type) {
	if (first_char.create_interaction) {
		auto interaction = first_char.create_interaction(first_char, second_char, type);
		if (interaction) {
			if (second_char.get_interacted_by) {
				second_char.get_interacted_by(second_char, first_char, type, *interaction);
			} else {
				LOG_WARN("Interaction generated but second character has no handler", m2g::pb::InteractionType_Name(type));
			}
		}
	}
}
bool m2::Character::has_item(m2g::pb::ItemType item_type) const {
    for (auto it = begin_items(); it != end_items(); ++it) {
        if (it->type() == item_type) {
            return true;
        }
    }
	return false;
}
bool m2::Character::has_item(m2g::pb::ItemCategory item_cat) const {
    for (auto it = begin_items(); it != end_items(); ++it) {
        if (it->category() == item_cat) {
            return true;
        }
    }
	return false;
}
size_t m2::Character::count_item(m2g::pb::ItemType item_type) const {
    size_t count = 0;
    for (auto it = begin_items(); it != end_items(); ++it) {
        if (it->type() == item_type) {
            ++count;
        }
    }
	return count;
}
size_t m2::Character::count_item(m2g::pb::ItemCategory item_cat) const {
    size_t count = 0;
    for (auto it = begin_items(); it != end_items(); ++it) {
        if (it->category() == item_cat) {
            ++count;
        }
    }
    return count;
}
bool m2::Character::use_item(const Iterator& item_it, float resource_multiplier) {
	if (item_it == end_items()) {
		return false;
	}
	if (item_it->usage() == pb::PASSIVE) {
		LOG_WARN("Attempted to use PASSIVE item_it", item_it->type());
		return false;
	}

	// Check if costs can be paid
	if (item_it->get_cost_count() == 1) {
		const auto cost = item_it->get_cost_by_index(0);
		const auto adjusted_cost_amount = cost.second * resource_multiplier;
		if (0.0f < adjusted_cost_amount && get_resource(cost.first) < adjusted_cost_amount) {
			return false;
		}
	} else if (1 < item_it->get_cost_count()) {
		// Merge costs
		auto resource_count = protobuf::enum_value_count<m2g::pb::ResourceType>();
		auto* merged_costs = (float*) alloca(resource_count * sizeof(float));
		for (size_t i = 0; i < item_it->get_cost_count(); ++i) {
			const auto cost = item_it->get_cost_by_index(i);
			merged_costs[protobuf::enum_index(cost.first)] += cost.second * resource_multiplier;
		}
		// Check if all costs are covered
		for (int i = 0; i < resource_count; ++i) {
			if (0.0f < merged_costs[i] && get_resource(protobuf::enum_value<m2g::pb::ResourceType>(i)) < merged_costs[i]) {
				return false;
			}
		}
	}

	// Pay the costs
	for (size_t i = 0; i < item_it->get_cost_count(); ++i) {
		const auto cost = item_it->get_cost_by_index(i);
		remove_resource(cost.first, cost.second * resource_multiplier);
	}
	// Get the benefits
	for (size_t i = 0; i < item_it->get_benefit_count(); ++i) {
		const auto benefit = item_it->get_benefit_by_index(i);
		add_resource(benefit.first, benefit.second * resource_multiplier);
	}
	if (item_it->usage() == pb::CONSUMABLE) {
		remove_item(item_it);
	}
	return true;
}

namespace {
	void tiny_character_iterator_incrementor(m2::Character::Iterator& it) {
		it.set(nullptr);
	}
}

m2::TinyCharacter::TinyCharacter(uint64_t object_id) : Character(object_id) {}
void m2::TinyCharacter::automatic_update() {
	if (_item && _item->usage() == pb::AUTOMATIC) {
		use_item(begin_items(), GAME.delta_time_s());
	}
}
m2::Character::Iterator m2::TinyCharacter::find_items(m2g::pb::ItemType item_type) {
	return {*this, tiny_character_iterator_incrementor, item_type, 0,
			_item && _item->type() == item_type ? _item.get() : nullptr};
}
m2::Character::Iterator m2::TinyCharacter::find_items(m2g::pb::ItemCategory cat) {
	return {*this, tiny_character_iterator_incrementor, cat, 0,
			_item && _item->category() == cat ? _item.get() : nullptr};
}
m2::Character::Iterator m2::TinyCharacter::begin_items() const {
	return {*this, tiny_character_iterator_incrementor, {}, 0, _item ? _item.get() : nullptr};
}
m2::Character::Iterator m2::TinyCharacter::end_items() const {
	return {*this, tiny_character_iterator_incrementor, {}, 0, nullptr};
}
void m2::TinyCharacter::add_item(SmartPointer<const Item>&& item) {
	_item = std::move(item);
	// Get acquire benefits
	for (size_t i = 0; i < _item->get_acquire_benefit_count(); ++i) {
		const auto benefit = _item->get_acquire_benefit_by_index(i);
		add_resource(benefit.first, benefit.second);
	}
	if (_item->use_on_acquire()) {
		use_item(begin_items());
	}
}
void m2::TinyCharacter::remove_item(const Iterator& item) {
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

void m2::full_character_iterator_incrementor(m2::Character::Iterator& it) {
	const auto& character = dynamic_cast<const m2::FullCharacter&>(it.character());
	auto curr_index = it.get_index();
	auto filter = it.get_filter();
	if (std::holds_alternative<std::monostate>(filter)) {
		if (curr_index + 1 < character._items.size()) {
			// Next item
			it.set_index(curr_index + 1);
			it.set(character._items[curr_index + 1].get());
			return;
		}
	} else if (std::holds_alternative<m2g::pb::ItemType>(filter)) {
		for (size_t i = curr_index + 1; i < character._items.size(); ++i) {
			if (character._items[i].get()->type() == std::get<m2g::pb::ItemType>(filter)) {
				// Found item
				it.set_index(i);
				it.set(character._items[i].get());
				return;
			}
		}
	} else if (std::holds_alternative<m2g::pb::ItemCategory>(filter)) {
		for (size_t i = curr_index + 1; i < character._items.size(); ++i) {
			if (character._items[i].get()->category() == std::get<m2g::pb::ItemCategory>(filter)) {
				// Found item
				it.set_index(i);
				it.set(character._items[i].get());
				return;
			}
		}
	} else {
		throw M2FATAL("Invalid iterator filter");
	}
	// Item not found
	it.set(nullptr);
}

m2::FullCharacter::FullCharacter(uint64_t object_id) : Character(object_id) {}
void m2::FullCharacter::automatic_update() {
	for (auto it = begin_items(); it != end_items(); ++it) {
		if (it->usage() == pb::AUTOMATIC) {
			use_item(it, GAME.delta_time_s());
		}
	}
}
m2::Character::Iterator m2::FullCharacter::find_items(m2g::pb::ItemType item_type) {
	for (size_t i = 0; i < _items.size(); ++i) {
		const auto& item = _items[i];
		if (item.get()->type() == item_type) {
			return {*this, full_character_iterator_incrementor, item_type, i, item.get()};
		}
	}
	return end_items();
}
m2::Character::Iterator m2::FullCharacter::find_items(m2g::pb::ItemCategory cat) {
	for (size_t i = 0; i < _items.size(); ++i) {
		const auto& item = _items[i];
		if (item.get()->category() == cat) {
			return {*this, full_character_iterator_incrementor, cat, i, item.get()};
		}
	}
	return end_items();
}
m2::Character::Iterator m2::FullCharacter::begin_items() const {
	if (!_items.empty()) {
		return {*this, full_character_iterator_incrementor, {}, 0, _items.front().get()};
	}
	return end_items();
}
m2::Character::Iterator m2::FullCharacter::end_items() const {
	return {*this, full_character_iterator_incrementor, {}, 0, nullptr};
}
void m2::FullCharacter::add_item(SmartPointer<const Item>&& item) {
	_items.emplace_back(std::move(item));
	// Get acquire benefits
	for (size_t i = 0; i < _items.back()->get_acquire_benefit_count(); ++i) {
		const auto benefit = _items.back()->get_acquire_benefit_by_index(i);
		add_resource(benefit.first, benefit.second);
	}
	if (_items.back()->use_on_acquire()) {
		use_item(Iterator{*this, full_character_iterator_incrementor, {}, _items.size() - 1, _items.back().get()});
	}
}
void m2::FullCharacter::remove_item(const Iterator& item) {
	if (item != end_items()) {
		auto it = _items.cbegin();
		std::advance(it, item.get_index());
		_items.erase(it);
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
	return protobuf::enum_index<m2g::pb::ResourceType>(resource_type);
}

m2::Character& m2::get_character_base(CharacterVariant& v) {
	return std::visit(m2::overloaded {
			[](TinyCharacter& v) -> Character& { return v; },
			[](FullCharacter& v) -> Character& { return v; },
	}, v);
}
