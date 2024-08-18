#include "m2/game/object/Pointer.h"

#include "m2/Game.h"
#include "m2/Object.h"

m2::Id m2::obj::create_pointer() {
    auto it = m2::create_object({});

	it->add_graphic().pre_draw = [](MAYBE m2::Graphic& gfx) {
		gfx.owner().position = M2_GAME.mouse_position_world_m();
	};

    auto& lig = it->add_light();
	//lig.dynamic_category_bits = box2d::FIXTURE_CATEGORY_OBSTACLE;
    lig.radius_m = 3.0f;

    return M2_LEVEL.pointer_id = it.id();
}
