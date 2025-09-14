#include "m2/game/object/Pointer.h"

#include "m2/Game.h"
#include "m2/Object.h"

m2::Id m2::obj::CreatePointer() {
    const auto it = CreateObject();

	it->AddGraphic(pb::FlatGraphicsLayer::FOREGROUND_FLAT).preDraw = [](Graphic& gfx_, const Stopwatch::Duration&) {
		gfx_.position = M2_GAME.MousePositionWorldM();
	};

    auto& lig = it->AddLight();
    lig.radiusM = 3.0f;

    return M2_LEVEL.pointer_id = it.GetId();
}
