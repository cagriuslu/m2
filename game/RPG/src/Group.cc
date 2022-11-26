#include <m2g/Group.h>
#include <rpg/group/ItemGroup.h>

m2::Group* m2g::create_group(pb::GroupType group_type) {
	switch (group_type) {
		case pb::GroupType::CONSUMABLE_ITEM_HP20: {
			auto* g = new rpg::ItemGroup();
			g->add_item(pb::ITEM_CONSUMABLE_POTION_20HP);
			return g;
		}
		default:
			return nullptr;
	}
}
