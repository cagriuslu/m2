#ifndef IMPL_CONSUMABLEDROP_H
#define IMPL_CONSUMABLEDROP_H

#include "rpg/Consumable.h"
#include "m2/Object.h"
#include "m2/Def.h"

namespace obj {
	struct ConsumableDrop : public m2::ObjectImpl {
		const itm::ConsumableBlueprint& blueprint;

		explicit ConsumableDrop(const itm::ConsumableBlueprint& blueprint);

		static M2Err init(m2::Object& obj, const itm::ConsumableBlueprint& blueprint, m2::Vec2f pos);
	};
}

#endif //IMPL_CONSUMABLEDROP_H
