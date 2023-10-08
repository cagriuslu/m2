#include <m2/object/Pixel.h>
#include <m2/Game.h>

m2::Id m2::obj::create_pixel(const VecF &pos, SDL_Color color) {
	auto [obj, id] = create_object(pos);

	auto& gfx = obj.add_terrain_graphic();
	gfx.on_draw = [=](Graphic& gfx) {
		auto screen_origin_to_position_px_vec = screen_origin_to_position_px(gfx.parent().position);
		auto dst_rect = SDL_Rect{
				(int)roundf(screen_origin_to_position_px_vec.x) - (GAME.dimensions().ppm / 2),
				(int)roundf(screen_origin_to_position_px_vec.y) - (GAME.dimensions().ppm / 2),
				GAME.dimensions().ppm,
				GAME.dimensions().ppm
		};
		SDL_SetRenderDrawColor(GAME.renderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(GAME.renderer, &dst_rect);
	};

	return id;
}
