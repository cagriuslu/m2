#include <m2/component/Character.h>
#include <m2/Game.hh>
#include <algorithm>

m2::CharacterBase::CharacterBase(uint64_t object_id) : Component(object_id) {}
bool m2::CharacterBase::use_item(m2g::pb::ItemType item_type) {
	if (!has_item(item_type)) {
		return false;
	}
	const auto& item = GAME.items[item_type];
	auto apply_item = [&]() {
		for (const auto& resources_it : item.resources()) {
			this->add_resource(resources_it.type(), static_cast<float>(resources_it.amount()));
		}
	};
	switch (item.usage()) {
		case pb::CONSUMABLE:
			apply_item();
			remove_item(item_type);
			break;
		case pb::REUSABLE:
		case pb::PASSIVE:
			apply_item();
			break;
		default:
			break;
	}
	return true;
}
bool m2::CharacterBase::use_item_if(m2g::pb::ItemType item_type) {
	if (!has_item(item_type)) {
		return false;
	}
	bool enough = true;
	for (const auto& resources_it : GAME.items[item_type].resources()) {
		if (resources_it.amount() < 0.0f && this->get_resource(resources_it.type()) < resources_it.amount()) {
			enough = false;
		}
	}
	if (enough) {
		return use_item(item_type);
	}
	return false;
}

m2::TinyCharacter::TinyCharacter(uint64_t object_id) : CharacterBase(object_id) {}
bool m2::TinyCharacter::has_item(m2g::pb::ItemType item_type) const {
	return item && *item == item_type;
}
size_t m2::TinyCharacter::count_item(m2g::pb::ItemType item_type) const {
	return has_item(item_type) ? 1 : 0;
}
void m2::TinyCharacter::add_item(m2g::pb::ItemType item_type) {
	item = item_type;
}
bool m2::TinyCharacter::remove_item(m2g::pb::ItemType item_type) {
	if (has_item(item_type)) {
		item = {};
		return true;
	}
	return false;
}
bool m2::TinyCharacter::clear_item(m2g::pb::ItemType item_type) {
	return remove_item(item_type);
}
bool m2::TinyCharacter::has_resource(m2g::pb::ResourceType resource_type) const {
	return resource && resource->first == resource_type && 0.0f < resource->second;
}
float m2::TinyCharacter::get_resource(m2g::pb::ResourceType resource_type) const {
	if (resource && resource->first == resource_type) {
		return static_cast<float>(resource->second);
	}
	return {};
}
float m2::TinyCharacter::add_resource(m2g::pb::ResourceType resource_type, float amount) {
	if (resource && resource->first == resource_type) {
		auto new_amount = resource->second + amount;
		new_amount = (new_amount < 0.0f) ? 0.0f : new_amount;
		resource->second = new_amount;
		return static_cast<float>(new_amount);
	} else if (0.0f < amount) {
		resource = std::make_pair(resource_type, amount);
		return amount;
	} else {
		resource = std::make_pair(resource_type, 0.0f);
		return 0.0f;
	}
}
float m2::TinyCharacter::remove_resource(m2g::pb::ResourceType resource_type, float amount) {
	return add_resource(resource_type, -amount);
}
bool m2::TinyCharacter::remove_resource_if(m2g::pb::ResourceType resource_type, float amount) {
	if (resource && resource->first == resource_type) {
		auto new_amount = resource->second - amount;
		if (new_amount < 0.0f) {
			return false;
		}
		resource->second = new_amount;
		return true;
	} else if (amount < 0.0f) {
		resource = std::make_pair(resource_type, amount);
		return true;
	} else {
		return false;
	}
}
bool m2::TinyCharacter::clear_resource(m2g::pb::ResourceType resource_type) {
	if (resource && resource->first == resource_type) {
		resource = {};
		return true;
	} else {
		return false;
	}
}
bool m2::TinyCharacter::clear_resource_if(m2g::pb::ResourceType resource_type, float amount) {
	if (resource && resource->first == resource_type) {
		auto new_amount = resource->second - amount;
		if (new_amount < 0.0f) {
			return false;
		}
		resource = {};
		return true;
	} else {
		return false;
	}
}


m2::FullCharacter::FullCharacter(uint64_t object_id) : CharacterBase(object_id) {}
bool m2::FullCharacter::has_item(m2g::pb::ItemType item_type) const {
	return items.find(item_type) != items.end();
}
size_t m2::FullCharacter::count_item(m2g::pb::ItemType item_type) const {
	return items.count(item_type);
}
void m2::FullCharacter::add_item(m2g::pb::ItemType item_type) {
	items.emplace(item_type);
}
bool m2::FullCharacter::remove_item(m2g::pb::ItemType item_type) {
	auto it = items.find(item_type);
	if (it != items.end()) {
		items.erase(it);
		return true;
	}
	return false;
}
bool m2::FullCharacter::clear_item(m2g::pb::ItemType item_type) {
	return items.erase(item_type);
}
bool m2::FullCharacter::has_resource(m2g::pb::ResourceType resource_type) const {
	return resources.count(resource_type);
}
float m2::FullCharacter::get_resource(m2g::pb::ResourceType resource_type) const {
	auto it = resources.find(resource_type);
	if (it != resources.end()) {
		return it->second;
	}
	return 0.0f;
}
float m2::FullCharacter::add_resource(m2g::pb::ResourceType resource_type, float amount) {
	auto it = resources.find(resource_type);
	if (it != resources.end()) {
		auto new_amount = it->second + amount;
		new_amount = (new_amount < 0.0f) ? 0.0f : new_amount;
		it->second = new_amount;
		return static_cast<float>(new_amount);
	} else if (0.0f < amount) {
		resources[resource_type] = amount;
		return amount;
	} else {
		return 0.0f;
	}
}
float m2::FullCharacter::remove_resource(m2g::pb::ResourceType resource_type, float amount) {
	return add_resource(resource_type, -amount);
}
bool m2::FullCharacter::remove_resource_if(m2g::pb::ResourceType resource_type, float amount) {
	auto it = resources.find(resource_type);
	if (it != resources.end()) {
		auto new_amount = it->second - amount;
		if (new_amount < 0.0f) {
			return false;
		}
		it->second = new_amount;
		return true;
	} else if (amount < 0.0f) {
		resources[resource_type] = -amount;
		return true;
	} else {
		return false;
	}
}
bool m2::FullCharacter::clear_resource(m2g::pb::ResourceType resource_type) {
	auto it = resources.find(resource_type);
	if (it != resources.end()) {
		resources.erase(it);
		return true;
	} else {
		return false;
	}
}
bool m2::FullCharacter::clear_resource_if(m2g::pb::ResourceType resource_type, float amount) {
	auto it = resources.find(resource_type);
	if (it != resources.end()) {
		auto new_amount = it->second - amount;
		if (new_amount < 0.0f) {
			return false;
		}
		resources.erase(it);
		return true;
	} else {
		return false;
	}
}

m2::CharacterBase& m2::get_character_base(CharacterVariant& v) {
	return std::visit(m2::overloaded {
			[](TinyCharacter& v) -> CharacterBase& { return v; },
			[](FullCharacter& v) -> CharacterBase& { return v; },
	}, v);
}
