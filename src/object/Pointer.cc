#include <m2/object/Pointer.h>
#include <m2/Object.h>
#include "m2/Game.h"

m2::Id m2::obj::create_pointer() {
    auto obj_pair = m2::create_object({});
	auto& obj = obj_pair.first;
	auto obj_id = obj_pair.second;

	obj.add_graphic().pre_draw = [&obj](MAYBE m2::Graphic& gfx) {
		obj.position = GAME.mouse_position_world_m();
	};

    auto& lig = obj.add_light();
	//lig.dynamic_category_bits = box2d::FIXTURE_CATEGORY_OBSTACLE;
    lig.radius_m = 3.0f;

	LEVEL.pointer_id = obj_id;
    return obj_pair.second;
}
