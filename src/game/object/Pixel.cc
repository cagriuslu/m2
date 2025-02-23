#include "m2/game/object/Pixel.h"

#include "m2/Game.h"

m2::Id m2::obj::create_pixel(const VecF &pos, SDL_Color color) {
	auto it = create_object(pos);

	auto& gfx = it->add_terrain_graphic(BackgroundLayer::L0);
	gfx.onDraw = [=](Graphic& gfx) {
		auto screen_origin_to_position_px_vec = ScreenOriginToPositionVecPx(gfx.owner().position);
		auto dst_rect = SDL_Rect{
				iround(screen_origin_to_position_px_vec.x - (M2_GAME.Dimensions().OutputPixelsPerMeter() / 2.0f)),
				iround(screen_origin_to_position_px_vec.y - (M2_GAME.Dimensions().OutputPixelsPerMeter() / 2.0f)),
				iround(M2_GAME.Dimensions().OutputPixelsPerMeter()),
				iround(M2_GAME.Dimensions().OutputPixelsPerMeter())
		};
		SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(M2_GAME.renderer, &dst_rect);
	};

	return it.Id();
}
