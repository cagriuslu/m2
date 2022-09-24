#ifndef RPG_ITEMGROUP_H
#define RPG_ITEMGROUP_H

#include <m2/Group.h>
#include <ItemType.pb.h>
#include <deque>
#include <optional>

namespace rpg {
	class ItemGroup : public m2::Group {
		std::deque<m2g::pb::ItemType> items;

	public:
		ItemGroup() = default;

		void add_item(m2g::pb::ItemType item);
		std::optional<m2g::pb::ItemType> pop_item();
	};
}

#endif //RPG_ITEMGROUP_H
