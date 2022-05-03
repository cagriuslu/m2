#ifndef IMPL_LEVELBLUEPRINT_H
#define IMPL_LEVELBLUEPRINT_H

#include "m2/LevelBlueprint.h"

namespace lvl {
	enum ObjectGroup : m2::GroupTypeID {
		NO_GROUP [[maybe_unused]] = 0,
		CONSUMABLE_RESOURCE_GROUP,
		CONSUMABLE_RESOURCE_GROUP_HP,
		CONSUMABLE_RESOURCE_GROUP_N
	};

    extern const m2::LevelBlueprint sp_000;
}

#endif //IMPL_LEVELBLUEPRINT_H
