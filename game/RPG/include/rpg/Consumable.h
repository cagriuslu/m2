#ifndef IMPL_CONSUMABLE_H
#define IMPL_CONSUMABLE_H

#include "rpg/Attribute.h"
#include "m2/SpriteBlueprint.h"
#include <list>

namespace itm {
	struct Consumable {
		m2::SpriteIndex drop_sprite_index;
		std::list<std::pair<chr::Attribute, float>> buffs;
	};

	extern const Consumable health_drop_20;
}

#endif //IMPL_CONSUMABLE_H
