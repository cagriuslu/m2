#include "m2/game/object/Line.h"

m2::Id m2::obj::create_vertical_line(const float x, const RGBA& color) {
	const auto it = CreateObject(VecF{x, 0.0f});

	auto& gfx = it->AddGraphic(ForegroundDrawLayer::F0_BOTTOM);
	gfx.onDraw = [x, color](Graphic &) {
		Graphic::DrawVerticalLine(x, color);
	};

	return it.GetId();
}

m2::Id m2::obj::create_horizontal_line(const float y, const RGBA& color) {
	const auto it = CreateObject(VecF{0.0f, y});

	auto& gfx = it->AddGraphic(ForegroundDrawLayer::F0_BOTTOM);
	gfx.onDraw = [y, color](Graphic &) {
		Graphic::DrawHorizontalLine(y, color);
	};

	return it.GetId();
}
