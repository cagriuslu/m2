#include "m2/game/object/Line.h"

m2::Id m2::obj::create_vertical_line(float x, const RGBA& color) {
	auto it = CreateObject(VecF{x, 0.0f});

	auto& gfx = it->AddGraphic();
	gfx.onDraw = [x, color](MAYBE m2::Graphic &gfx) {
		Graphic::DrawVerticalLine(x, color);
	};

	return it.GetId();
}

m2::Id m2::obj::create_horizontal_line(float y, const RGBA& color) {
	auto it = CreateObject(VecF{0.0f, y});

	auto& gfx = it->AddGraphic();
	gfx.onDraw = [y, color](MAYBE m2::Graphic &gfx) {
		Graphic::DrawHorizontalLine(y, color);
	};

	return it.GetId();
}
