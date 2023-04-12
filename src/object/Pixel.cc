#include <m2/object/Pixel.h>
#include <m2/Game.h>

m2::Id m2::obj::create_pixel(const Vec2f &pos, SDL_Color color) {
	auto [obj, id] = create_object(pos);

	auto& gfx = obj.add_terrain_graphic();
	gfx.on_draw = [=](Graphic& gfx) {
		auto screen_origin_to_sprite_center_px = gfx.screen_origin_to_sprite_center_px();
		auto dst_rect = SDL_Rect{
				(int)roundf(screen_origin_to_sprite_center_px.x) - (GAME.game_ppm / 2),
				(int)roundf(screen_origin_to_sprite_center_px.y) - (GAME.game_ppm / 2),
				GAME.game_ppm,
				GAME.game_ppm
		};
		SDL_SetRenderDrawColor(GAME.sdlRenderer, color.r, color.g, color.b, color.a);
		SDL_RenderFillRect(GAME.sdlRenderer, &dst_rect);
	};

	return id;
}
