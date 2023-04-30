#include <m2/object/Origin.h>
#include <m2/Game.h>

std::pair<m2::Object&, m2::Id> m2::obj::create_origin() {
	auto obj_pair = create_object(Vec2f{});
	auto& origin = obj_pair.first;

	auto& gfx = origin.add_graphic();
	gfx.on_draw = [](m2::Graphic &gfx) {
		SDL_SetRenderDrawColor(GAME.renderer, 255, 0, 0, 255);
		auto draw_position = Vec2i{screen_origin_to_position_px(gfx.parent().position)};
		SDL_RenderDrawLine(GAME.renderer, draw_position.x - 9, draw_position.y - 9, draw_position.x + 10, draw_position.y + 10);
		SDL_RenderDrawLine(GAME.renderer, draw_position.x - 9, draw_position.y + 9, draw_position.x + 10, draw_position.y - 10);
	};

	return obj_pair;
}
