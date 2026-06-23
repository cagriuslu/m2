#include "m2/game/object/Origin.h"

#include "m2/Game.h"

m2::Id m2::obj::CreateOrigin() {
	const auto it = CreateObject();

	auto& gfx = it->AddGraphic(m2g::pb::UPRIGHT_GRAPHICS_DEFAULT_LAYER);
	gfx.onDraw = [](const Graphic &gfx_) {
		Graphic::DrawCross(gfx_.position, RGBA{255, 0, 0, 255});
	};

	return it.GetId();
}
