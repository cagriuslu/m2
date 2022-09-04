#include <m2/object/Origin.h>
#include <m2/Game.hh>

std::pair<m2::Object&, m2::ID> m2::obj::create_origin() {
	auto obj_pair = create_object(Vec2f{});
	auto& origin = obj_pair.first;

	auto& gfx = origin.add_graphic();
	gfx.on_draw = [](m2::comp::Graphic &gfx) {
		SDL_SetRenderDrawColor(GAME.sdlRenderer, 255, 0, 0, 255);
		auto draw_position = gfx.offset_from_screen_origin_px();
		SDL_RenderDrawLine(GAME.sdlRenderer, draw_position.x - 9, draw_position.y - 9, draw_position.x + 10, draw_position.y + 10);
		SDL_RenderDrawLine(GAME.sdlRenderer, draw_position.x - 9, draw_position.y + 9, draw_position.x + 10, draw_position.y - 10);
	};

	return obj_pair;
}
