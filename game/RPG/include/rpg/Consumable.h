#ifndef IMPL_CONSUMABLE_H
#define IMPL_CONSUMABLE_H

#include "rpg/Attribute.h"
#include "m2/SpriteBlueprint.h"
#include <list>

namespace itm {
	struct ConsumableBlueprint;
	struct ConsumableState {
		const ConsumableBlueprint& blueprint;

		explicit ConsumableState(const ConsumableBlueprint& blueprint);
	};

	struct ConsumableBlueprint {
		m2::SpriteIndex drop_sprite_index;
		std::list<std::pair<chr::Attribute, float>> buffs;
		float ttl_s; // 0:instantaneous, +inf:endless
	};

	extern const ConsumableBlueprint health_drop_20;
}

#endif //IMPL_CONSUMABLE_H
