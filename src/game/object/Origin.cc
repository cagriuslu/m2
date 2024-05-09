#include "m2/game/object/Origin.h"

#include "m2/Game.h"

m2::Id m2::obj::create_origin() {
	auto it = create_object({});

	auto& gfx = it->add_graphic();
	gfx.on_draw = [](m2::Graphic &gfx) {
		Graphic::draw_cross(gfx.parent().position, SDL_Color{255, 0, 0, 255});
	};

	return it.id();
}
