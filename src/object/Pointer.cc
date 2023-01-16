#include <m2/object/Pointer.h>
#include <m2/Object.h>
#include "m2/Game.h"

std::pair<m2::Object&, m2::Id> m2::obj::create_pointer() {
    auto obj_pair = m2::create_object({});
	auto& obj = obj_pair.first;
	auto obj_id = obj_pair.second;

	obj.add_graphic().pre_draw = [&obj](MAYBE m2::Graphic& gfx) {
		obj.position = GAME.mousePositionWRTGameWorld_m;
	};

    auto& lig = obj.add_light();
    lig.radius_m = 3.0f;

    GAME.pointerId = obj_id;
    return obj_pair;
}
