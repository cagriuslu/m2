#ifndef M2_CHARACTER_H
#define M2_CHARACTER_H

#include "../Component.h"
#include <Item.pb.h>
#include <vector>
#include <functional>

namespace m2 {
	struct Character : public Component {
		std::function<void(Character&)> update;
		std::function<void(Character&, Character&)> interact;

		[[nodiscard]] bool has_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] size_t count_item(m2g::pb::ItemType item_type) const;
		[[nodiscard]] float get_resource(m2g::pb::ResourceType resource_type) const;

		void add_item(m2g::pb::ItemType item_type);
		float add_resource(m2g::pb::ResourceType resource_type, float amount);

		/// Returns true if successful
		bool remove_item(m2g::pb::ItemType item_type);
		/// Returns true if successful
		bool clear_item(m2g::pb::ItemType item_type);

		/// Returns remaining amount
		float remove_resource(m2g::pb::ResourceType resource_type, float amount);
		/// Returns true if successful
		bool clear_resource(m2g::pb::ResourceType resource_type);

		Character() = default;
		explicit Character(uint64_t object_id);

	private:
		std::vector<m2g::pb::ItemType> items;
		std::vector<pb::Resource> resources;
	};
}

#endif //M2_CHARACTER_H
