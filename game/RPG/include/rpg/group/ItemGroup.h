#pragma once
#include <m2/Group.h>
#include <m2g_ItemType.pb.h>
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
