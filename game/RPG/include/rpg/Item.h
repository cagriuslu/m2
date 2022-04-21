#ifndef IMPL_ITEM_H
#define IMPL_ITEM_H

#include "rpg/Attribute.h"
#include "m2/SpriteBlueprint.h"
#include <list>

namespace itm {
	struct ItemBlueprint;
	struct ItemState {
		const ItemBlueprint& blueprint;

		explicit ItemState(const ItemBlueprint& blueprint);
	};

	struct ItemBlueprint {
		m2::SpriteIndex drop_sprite_index;
		std::list<std::pair<chr::Attribute, float>> buffs;
		float ttl_s; // 0:instantaneous, +inf:endless
	};

	extern const ItemBlueprint health_drop_20;
}

#endif //IMPL_ITEM_H
