#include "m2/game/object/Origin.h"

#include "m2/Game.h"

m2::Id m2::obj::create_origin() {
	const auto it = CreateObject({});

	auto& gfx = it->AddGraphic(ForegroundDrawLayer::F0_BOTTOM);
	gfx.onDraw = [](const Graphic &gfx_) {
		Graphic::DrawCross(gfx_.Owner().position, SDL_Color{255, 0, 0, 255});
	};

	return it.GetId();
}
