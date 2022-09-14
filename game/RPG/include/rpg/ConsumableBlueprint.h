#ifndef IMPL_CONSUMABLEBLUEPRINT_H
#define IMPL_CONSUMABLEBLUEPRINT_H

#include "rpg/Attribute.h"
#include "m2/SpriteBlueprint.h"
#include <m2g/SpriteId.h>
#include <vector>

namespace itm {
	struct ConsumableBlueprint {
		m2g::SpriteId drop_sprite;
		std::vector<std::pair<chr::Attribute, float>> buffs;
	};

	extern const ConsumableBlueprint health_drop_20;
}

#endif //IMPL_CONSUMABLEBLUEPRINT_H
