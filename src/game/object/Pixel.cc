#include "m2/game/object/Pixel.h"

#include "m2/Game.h"

m2::Id m2::obj::create_pixel(const VecF &pos, SDL_Color color) {
	auto it = create_object(pos);

	auto& gfx = it->add_terrain_graphic(BackgroundLayer::L0);
	gfx.on_draw = [=](Graphic& gfx) {
		auto screen_origin_to_position_px_vec = screen_origin_to_position_dstpx(gfx.parent().position);
		auto dst_rect = SDL_Rect{
				(int)roundf(screen_origin_to_position_px_vec.x) - (M2_GAME.dimensions().ppm / 2),
				(int)roundf(screen_origin_to_position_px_vec.y) - (M2_GAME.dimensions().ppm / 2),
				M2_GAME.dimensions().ppm,
				M2_GAME.dimensions().ppm
		};
		SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(M2_GAME.renderer, &dst_rect);
	};

	return it.id();
}
