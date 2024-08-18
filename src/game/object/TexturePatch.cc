#include "m2/game/object/TexturePatch.h"

#include "m2/Game.h"

m2::Id m2::obj::create_texture_patch(const VecF& pos, SDL_Texture* texture, RectI rect, int ppm) {
	auto it = create_object(pos);

	auto src_rect = static_cast<SDL_Rect>(rect);

	auto& gfx = it->add_terrain_graphic(BackgroundLayer::L0);
	gfx.on_draw = [=, src_rect = src_rect](Graphic& _gfx) {
		auto texture_width_dstpx = rect.w * M2_GAME.dimensions().ppm / ppm;
		auto texture_height_dstpx = rect.h * M2_GAME.dimensions().ppm / ppm;

		auto screen_origin_to_position_dstpx_vec = screen_origin_to_position_dstpx(_gfx.owner().position);
		auto dst_rect = SDL_Rect{
		    (int)roundf(screen_origin_to_position_dstpx_vec.x) - (texture_width_dstpx / 2),
		    (int)roundf(screen_origin_to_position_dstpx_vec.y) - (texture_height_dstpx / 2), texture_width_dstpx,
		    texture_height_dstpx};
		SDL_RenderCopy(M2_GAME.renderer, texture, &src_rect, &dst_rect);
	};

	return it.id();
}
