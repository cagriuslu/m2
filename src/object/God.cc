#include <m2/object/God.h>
#include <m2/Game.hh>

std::pair<m2::Object&, m2::ID> m2::obj::create_god() {
	auto obj_pair = create_object(Vec2f{});
	auto& god = obj_pair.first;
	god.impl = std::make_unique<m2::obj::God>();

	auto& monitor = god.add_monitor();
	monitor.pre_phy = [&]([[maybe_unused]] m2::comp::Monitor& mon) {
		m2::Vec2f move_direction;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			move_direction.y += 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			move_direction.y += -1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			move_direction.x += 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			move_direction.x += -1.0f;
		}
		god.position += move_direction.normalize() * (GAME.deltaTicks_ms * .004f);
	};

	monitor.pre_gfx = [&]([[maybe_unused]] m2::comp::Monitor& mon) {
		// Draw gridlines
		SDL_SetRenderDrawColor(GAME.sdlRenderer, 127, 127, 127, 127);

		float yOffsetFromWholeMeter = god.position.y - floorf(god.position.y);
		float yOffsetFromWholeMeterInPixels = yOffsetFromWholeMeter * GAME.pixelsPerMeter;
		// Draw lines above god
		for (float y = yOffsetFromWholeMeterInPixels + (float) GAME.gameRect.y + (float) GAME.gameRect.h / 2.0f; (float) GAME.gameRect.y < y; y -= GAME.pixelsPerMeter) {
			SDL_RenderDrawLine(GAME.sdlRenderer, GAME.gameRect.x, y, GAME.gameRect.x + GAME.gameRect.w, y);
		}
		// Draw lines below god
		for (float y = yOffsetFromWholeMeterInPixels + (float) GAME.gameRect.y + (float) GAME.gameRect.h / 2.0f + GAME.pixelsPerMeter; y < (float)(GAME.gameRect.y + GAME.gameRect.h); y += GAME.pixelsPerMeter) {
			SDL_RenderDrawLine(GAME.sdlRenderer, GAME.gameRect.x, y, GAME.gameRect.x + GAME.gameRect.w, y);
		}
		float xOffsetFromWholeMeter = god.position.x - floorf(god.position.x);
		float xOffsetFromWholeMeterInPixels = xOffsetFromWholeMeter * GAME.pixelsPerMeter;
		// Draw lines left of god
		for (float x = xOffsetFromWholeMeterInPixels + (float) GAME.gameRect.x + (float) GAME.gameRect.w / 2.0f; (float) GAME.gameRect.x < x; x -= GAME.pixelsPerMeter) {
			SDL_RenderDrawLine(GAME.sdlRenderer, x, GAME.gameRect.y, x, GAME.gameRect.y + GAME.gameRect.h);
		}
		// Draw lines right of god
		for (float x = xOffsetFromWholeMeterInPixels + (float) GAME.gameRect.x + (float) GAME.gameRect.w / 2.0f + GAME.pixelsPerMeter; x < (float)(GAME.gameRect.x + GAME.gameRect.w); x += GAME.pixelsPerMeter) {
			SDL_RenderDrawLine(GAME.sdlRenderer, x, GAME.gameRect.y, x, GAME.gameRect.y + GAME.gameRect.h);
		}
	};

	GAME.playerId = obj_pair.second;
	return obj_pair;
}
