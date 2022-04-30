#ifndef IMPL_LEVELBLUEPRINT_H
#define IMPL_LEVELBLUEPRINT_H

#include "m2/LevelBlueprint.h"

namespace lvl {
	enum ObjectGroup : m2::GroupID {
		NO_GROUP [[maybe_unused]] = 0,
		CONSUMABLE_GROUP,
		CONSUMABLE_GROUP_HP,
		CONSUMABLE_GROUP_N
	};

    extern const m2::LevelBlueprint sp_000;
}

#endif //IMPL_LEVELBLUEPRINT_H
