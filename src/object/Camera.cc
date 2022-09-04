#include <m2/object/Camera.h>
#include "m2/Object.h"
#include "m2/Game.hh"

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

std::pair<m2::Object&, m2::ID> m2::obj::create_camera() {
    // Start at player's location
    auto* player = GAME.objects.get(GAME.playerId);
    auto obj_pair = create_object(player ? player->position : Vec2f{});
	auto& camera = obj_pair.first;
	camera.impl = std::make_unique<m2::obj::Camera>();

	auto& mon = camera.add_monitor();
	mon.post_phy = [&](MAYBE m2::comp::Monitor& monitor) {
		auto* camera_data = dynamic_cast<m2::obj::Camera*>(camera.impl.get());
		auto& player = GAME.objects[GAME.playerId];

		if (GAME.level->type() == Level::Type::SINGLE_PLAYER) {
			// Give an offset to the camera's location based on the position of the mouse
			m2::Vec2f offsetWRTScreenCenter = GAME.mousePositionWRTScreenCenter_m.ceil_length(OFFSET_LIMIT);
			camera_data->offset = camera_data->offset.lerp(offsetWRTScreenCenter, 0.5f * CAMERA_JUMP_RATIO);
			camera.position = camera.position.lerp(player.position + camera_data->offset, CAMERA_JUMP_RATIO);
		} else {
			camera.position = player.position;
		}
	};

	if (GAME.level->type() == Level::Type::EDITOR) {
		auto& gfx = camera.add_graphic();
		gfx.on_draw = [&](MAYBE comp::Graphic& gfx) {
			auto* camera_data = dynamic_cast<m2::obj::Camera*>(camera.impl.get());
			if (camera_data->draw_grid_lines) {
				auto offset_from_floored_position_m = camera.position - camera.position.floor();
				auto offset_from_floored_position_px = offset_from_floored_position_m * GAME.game_ppm;
				auto screen_center = Vec2i{GAME.windowRect.w / 2, GAME.windowRect.h / 2 };
				auto horizontal_line_y = [=](int index) -> int {
					return screen_center.y - (int)roundf(offset_from_floored_position_px.y) + index * (int)GAME.game_ppm;
				};
				auto vertical_line_x = [=](int index) -> int {
					return screen_center.x - (int)roundf(offset_from_floored_position_px.x) + index * (int)GAME.game_ppm;
				};
				// Draw horizontal lines
				SDL_SetRenderDrawColor(GAME.sdlRenderer, 127, 127, 255, 127);
				for (int i = 0, y = horizontal_line_y(i); y <= GAME.gameRect.y + GAME.gameRect.h; ++i, y = horizontal_line_y(i)) {
					SDL_RenderDrawLine(GAME.sdlRenderer, GAME.gameRect.x, y, GAME.gameRect.x + GAME.gameRect.w, y);
				}
				for (int i = -1, y = horizontal_line_y(i); GAME.gameRect.y <= y; --i, y = horizontal_line_y(i)) {
					SDL_RenderDrawLine(GAME.sdlRenderer, GAME.gameRect.x, y, GAME.gameRect.x + GAME.gameRect.w, y);
				}
				// Draw vertical lines
				for (int i = 0, x = vertical_line_x(i); x <= GAME.gameRect.x + GAME.gameRect.w; ++i, x = vertical_line_x(i)) {
					SDL_RenderDrawLine(GAME.sdlRenderer, x, GAME.gameRect.y, x, GAME.gameRect.y + GAME.gameRect.h);
				}
				for (int i = -1, x = vertical_line_x(i); GAME.gameRect.x <= x; --i, x = vertical_line_x(i)) {
					SDL_RenderDrawLine(GAME.sdlRenderer, x, GAME.gameRect.y, x, GAME.gameRect.y + GAME.gameRect.h);
				}
			}
		};
	}

    GAME.cameraId = obj_pair.second;
    return obj_pair;
}
