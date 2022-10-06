#include <m2/Item.h>
#include <m2/protobuf/Utils.h>
#include <m2/Exception.h>

std::vector<m2::pb::Item> m2::load_items(const std::string &items_path) {
	auto items = proto::json_file_to_message<pb::Items>(items_path);
	if (!items) {
		return {};
	}

	std::vector<pb::Item> items_vector(m2g::pb::ItemType_ARRAYSIZE);
	std::vector<bool> is_loaded(m2g::pb::ItemType_ARRAYSIZE);

	for (const auto& item : items->items()) {
		// Check if the item is already loaded
		if (is_loaded[item.type()]) {
			throw M2ERROR("Item has duplicate definition: " + std::to_string(item.type()));
		}
		// Load item
		items_vector[item.type()] = item;
		is_loaded[item.type()] = true;
	}

	// Check if every item is loaded
	const auto* item_type_desc = m2g::pb::ItemType_descriptor();
	for (int e = 0; e < item_type_desc->value_count(); ++e) {
		int value = item_type_desc->value(e)->number();
		if (!is_loaded[value]) {
			throw M2ERROR("Item is not defined: " + std::to_string(value));
		}
	}

	return items_vector;
}
