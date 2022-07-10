#include <m2/object/Pointer.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Def.h"

std::pair<m2::Object&, m2::ID> m2::obj::create_pointer() {
    auto obj_pair = m2::create_object({});
	auto& obj = obj_pair.first;
	auto obj_id = obj_pair.second;

    auto& monitor = obj.add_monitor();

    auto& lig = obj.add_light();
    lig.radius_m = 3.0f;

	monitor.pre_gfx = [&](MAYBE m2::comp::Monitor& mon) {
		obj.position = GAME.mousePositionWRTGameWorld_m;
	};

    GAME.pointerId = obj_id;
    return obj_pair;
}
