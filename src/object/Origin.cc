#include <m2/object/Origin.h>
#include <m2/Game.h>

m2::Id m2::obj::create_origin() {
	auto obj_pair = create_object(VecF{});
	auto& origin = obj_pair.first;

	auto& gfx = origin.add_graphic();
	gfx.on_draw = [](m2::Graphic &gfx) {
		Graphic::draw_cross(gfx.parent().position, SDL_Color{255, 0, 0, 255});
	};

	return obj_pair.second;
}
