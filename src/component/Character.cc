#include <m2/component/Character.h>
#include <algorithm>

m2::Character::Character(uint64_t object_id) : Component(object_id) {}

bool m2::Character::has_item(m2g::pb::ItemType item_type) const {
	return std::any_of(items.begin(), items.end(), [=](const auto& item) {
		return item == item_type;
	});
}
size_t m2::Character::count_item(m2g::pb::ItemType item_type) const {
	return std::count_if(items.begin(), items.end(), [=](const auto& item) {
		return item == item_type;
	});
}
float m2::Character::get_resource(m2g::pb::ResourceType resource_type) const {
	auto it = std::find_if(resources.begin(), resources.end(), [=](const auto& resource) {
		return resource.type() == resource_type;
	});
	if (it != resources.end()) {
		return static_cast<float>(it->amount());
	}
	return {};
}

void m2::Character::add_item(m2g::pb::ItemType item_type) {
	items.emplace_back(item_type);
}
float m2::Character::add_resource(m2g::pb::ResourceType resource_type, float amount) {
	auto it = std::find_if(resources.begin(), resources.end(), [=](const auto& item) {
		return item.type() == resource_type;
	});
	if (it != resources.end()) {
		it->set_amount(it->amount() + amount);
		return static_cast<float>(it->amount());
	} else {
		auto& new_resource = resources.emplace_back();
		new_resource.set_type(resource_type);
		new_resource.set_amount(amount);
		return amount;
	}
}

bool m2::Character::remove_item(m2g::pb::ItemType item_type) {
	auto it = std::find_if(items.begin(), items.end(), [=](const auto& item) {
		return item == item_type;
	});
	if (it != items.end()) {
		items.erase(it);
		return true;
	} else {
		return false;
	}
}
bool m2::Character::clear_item(m2g::pb::ItemType item_type) {
	auto removed = false;

	auto result = true;
	while (result) {
		result = remove_item(item_type);
		removed = result || removed;
	}

	return removed;
}
float m2::Character::remove_resource(m2g::pb::ResourceType resource_type, float amount) {
	auto it = std::find_if(resources.begin(), resources.end(), [=](const auto& item) {
		return item.type() == resource_type;
	});
	if (it != resources.end()) {
		auto remaining = it->amount() - amount;
		if (remaining < 0.0) {
			resources.erase(it);
			return {};
		} else {
			it->set_amount(remaining);
			return static_cast<float>(remaining);
		}
	} else {
		return {};
	}
}
bool m2::Character::clear_resource(m2g::pb::ResourceType resource_type) {
	auto it = std::find_if(resources.begin(), resources.end(), [=](const auto& item) {
		return item.type() == resource_type;
	});
	if (it != resources.end()) {
		resources.erase(it);
		return true;
	} else {
		return false;
	}
}
