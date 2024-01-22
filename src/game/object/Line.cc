#include "m2/game/object/Line.h"

m2::Id m2::obj::create_vertical_line(float x, SDL_Color color) {
	auto [obj, id] = create_object(VecF{x, 0.0f});

	auto& gfx = obj.add_graphic();
	gfx.on_draw = [x, color](MAYBE m2::Graphic &gfx) {
		Graphic::draw_vertical_line(x, color);
	};

	return id;
}

m2::Id m2::obj::create_horizontal_line(float y, SDL_Color color) {
	auto [obj, id] = create_object(VecF{0.0f, y});

	auto& gfx = obj.add_graphic();
	gfx.on_draw = [y, color](MAYBE m2::Graphic &gfx) {
		Graphic::draw_horizontal_line(y, color);
	};

	return id;
}
