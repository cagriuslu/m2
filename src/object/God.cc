#include <m2/object/God.h>
#include <m2/Game.hh>

std::pair<m2::Object&, m2::ID> m2::obj::create_god() {
	auto obj_pair = create_object(Vec2f{});
	auto& god = obj_pair.first;
	god.impl = std::make_unique<m2::obj::God>();

	auto& monitor = god.add_monitor();
	monitor.pre_phy = [&](MAYBE m2::comp::Monitor& mon) {
		m2::Vec2f move_direction;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			move_direction.y -= 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			move_direction.y += 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			move_direction.x += 1.0f;
		}
		god.position += move_direction.normalize() * ((float)GAME.deltaTicks_ms * .004f);
	};

	auto& gfx = god.add_graphic();
	gfx.on_draw = [&](MAYBE m2::comp::Graphic& gfx) {
		if (GAME.level->editor_grid_lines) {
			// Draw gridlines
			SDL_SetRenderDrawColor(GAME.sdlRenderer, 127, 127, 127, 127);
			float yOffsetFromWholeMeter = god.position.y - floor(god.position.y);
			float yOffsetFromWholeMeterInPixels = yOffsetFromWholeMeter * GAME.game_ppm;
			// Draw lines above god
			for (float y = -yOffsetFromWholeMeterInPixels + (float) GAME.gameRect.y + (float) GAME.gameRect.h / 2.0f; (float) GAME.gameRect.y < y; y -= GAME.game_ppm) {
				SDL_RenderDrawLine(GAME.sdlRenderer, GAME.gameRect.x, (int)y, GAME.gameRect.x + GAME.gameRect.w, (int)y);
			}
			// Draw lines below god
			for (float y = -yOffsetFromWholeMeterInPixels + (float) GAME.gameRect.y + (float) GAME.gameRect.h / 2.0f + GAME.game_ppm; y < (float)(GAME.gameRect.y + GAME.gameRect.h); y += GAME.game_ppm) {
				SDL_RenderDrawLine(GAME.sdlRenderer, GAME.gameRect.x, (int)y, GAME.gameRect.x + GAME.gameRect.w, (int)y);
			}
			float xOffsetFromWholeMeter = god.position.x - floor(god.position.x);
			float xOffsetFromWholeMeterInPixels = xOffsetFromWholeMeter * GAME.game_ppm;
			// Draw lines left of god
			for (float x = -xOffsetFromWholeMeterInPixels + (float) GAME.gameRect.x + (float) GAME.gameRect.w / 2.0f; (float) GAME.gameRect.x < x; x -= GAME.game_ppm) {
				SDL_RenderDrawLine(GAME.sdlRenderer, (int)x, GAME.gameRect.y, (int)x, GAME.gameRect.y + GAME.gameRect.h);
			}
			// Draw lines right of god
			for (float x = -xOffsetFromWholeMeterInPixels + (float) GAME.gameRect.x + (float) GAME.gameRect.w / 2.0f + GAME.game_ppm; x < (float)(GAME.gameRect.x + GAME.gameRect.w); x += GAME.game_ppm) {
				SDL_RenderDrawLine(GAME.sdlRenderer, (int)x, GAME.gameRect.y, (int)x, GAME.gameRect.y + GAME.gameRect.h);
			}
		}
	};

	GAME.playerId = obj_pair.second;
	return obj_pair;
}
