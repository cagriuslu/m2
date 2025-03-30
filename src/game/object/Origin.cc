#include "m2/game/object/Origin.h"

#include "m2/Game.h"

m2::Id m2::obj::create_origin() {
	auto it = CreateObject({});

	auto& gfx = it->AddGraphic();
	gfx.onDraw = [](m2::Graphic &gfx) {
		Graphic::DrawCross(gfx.Owner().position, SDL_Color{255, 0, 0, 255});
	};

	return it.GetId();
}
