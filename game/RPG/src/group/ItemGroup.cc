#include <rpg/group/ItemGroup.h>
#include <m2/M2.h>

void rpg::ItemGroup::add_item(m2g::pb::ItemType item) {
	items.emplace_back(item);
}
std::optional<m2g::pb::ItemType> rpg::ItemGroup::pop_item() {
	if (!items.empty() && m2::rand(member_count()) == 0) {
		auto item = items.front();
		items.pop_front();
		return item;
	}
	return {};
}
