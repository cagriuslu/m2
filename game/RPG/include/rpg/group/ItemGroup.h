#ifndef RPG_ITEMGROUP_H
#define RPG_ITEMGROUP_H

#include <m2/Group.h>
#include <ItemType.pb.h>
#include <deque>
#include <optional>

namespace rpg {
	class ItemGroup : public m2::Group {
		std::optional<m2g::pb::ItemType> _item;

	public:
		ItemGroup(std::initializer_list<std::pair<m2g::pb::ItemType, float>>&& item_probabilities);

		std::optional<m2g::pb::ItemType> pop_item();
	};
}

#endif //RPG_ITEMGROUP_H
