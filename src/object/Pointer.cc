#include <m2/object/pointer.h>
#include <m2/object.hh>
#include "m2/Game.hh"
#include "m2/Def.hh"

void pointer_pre_graphics(Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	obj.position = GAME.mousePositionInWorld_m;
}

std::pair<m2::object::Object&, ID> m2::object::pointer::create() {
    auto obj_pair = object::create({});

    auto& mon = obj_pair.first.add_monitor();
    mon.preGraphics = pointer_pre_graphics;

    auto& lig = obj_pair.first.add_light();
    lig.radius_m = 3.0f;

    GAME.pointerId = obj_pair.second;
    return obj_pair;
}
