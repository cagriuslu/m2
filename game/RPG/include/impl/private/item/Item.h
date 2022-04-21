#ifndef IMPL_ITEM_H
#define IMPL_ITEM_H

#include <impl/private/character/Attribute.h>
#include <m2/SpriteBlueprint.h>
#include <list>

namespace impl::item {
	struct ItemBlueprint;
	struct ItemState {
		const ItemBlueprint& blueprint;

		explicit ItemState(const ItemBlueprint& blueprint);
	};

	struct ItemBlueprint {
		m2::SpriteIndex drop_sprite_index;
		std::list<std::pair<character::Attribute, float>> buffs;
		float ttl_s; // 0:instantaneous, +inf:endless
	};

	extern const ItemBlueprint health_drop_20;
}

#endif //IMPL_ITEM_H
