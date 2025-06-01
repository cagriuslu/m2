#include "m2/game/object/Pointer.h"

#include "m2/Game.h"
#include "m2/Object.h"

m2::Id m2::obj::create_pointer() {
    const auto it = CreateObject({});

	it->AddGraphic(ForegroundDrawLayer::F0_BOTTOM).preDraw = [](const Graphic& gfx_) {
		gfx_.Owner().position = M2_GAME.MousePositionWorldM();
	};

    auto& lig = it->AddLight();
	//lig.dynamic_category_bits = FIXTURE_CATEGORY_OBSTACLE;
    lig.radiusM = 3.0f;

    return M2_LEVEL.pointer_id = it.GetId();
}
