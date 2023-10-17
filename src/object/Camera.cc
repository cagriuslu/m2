#include <m2/object/Camera.h>
#include "m2/Object.h"
#include "m2/Game.h"

#include <m2/object/Line.h>

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

namespace {
	void draw_grid_lines(const m2::VecF& camera_position) {
		auto grid_offset = -0.5f; // Move grid to top left by 0.5f
		auto offset_from_nearest_integer_position_m = camera_position - (camera_position.floor() + grid_offset);
		auto offset_from_nearest_integer_position_px = offset_from_nearest_integer_position_m * GAME.dimensions().ppm;
		auto screen_center = m2::VecI{GAME.dimensions().window.w / 2, GAME.dimensions().window.h / 2 };
		auto horizontal_line_y = [=](int index) -> int {
			return screen_center.y - (int)roundf(offset_from_nearest_integer_position_px.y) + index * GAME.dimensions().ppm;
		};
		auto vertical_line_x = [=](int index) -> int {
			return screen_center.x - (int)roundf(offset_from_nearest_integer_position_px.x) + index * GAME.dimensions().ppm;
		};
		// Draw horizontal lines
		SDL_SetRenderDrawColor(GAME.renderer, 127, 127, 255, 127);
		for (int i = 0, y = horizontal_line_y(i); y <= GAME.dimensions().game.y + GAME.dimensions().game.h; ++i, y = horizontal_line_y(i)) {
			SDL_RenderDrawLine(GAME.renderer, GAME.dimensions().game.x, y, GAME.dimensions().game.x + GAME.dimensions().game.w, y);
		}
		for (int i = -1, y = horizontal_line_y(i); GAME.dimensions().game.y <= y; --i, y = horizontal_line_y(i)) {
			SDL_RenderDrawLine(GAME.renderer, GAME.dimensions().game.x, y, GAME.dimensions().game.x + GAME.dimensions().game.w, y);
		}
		// Draw vertical lines
		for (int i = 0, x = vertical_line_x(i); x <= GAME.dimensions().game.x + GAME.dimensions().game.w; ++i, x = vertical_line_x(i)) {
			SDL_RenderDrawLine(GAME.renderer, x, GAME.dimensions().game.y, x, GAME.dimensions().game.y + GAME.dimensions().game.h);
		}
		for (int i = -1, x = vertical_line_x(i); GAME.dimensions().game.x <= x; --i, x = vertical_line_x(i)) {
			SDL_RenderDrawLine(GAME.renderer, x, GAME.dimensions().game.y, x, GAME.dimensions().game.y + GAME.dimensions().game.h);
		}
	}
}

m2::Id m2::obj::create_camera() {
    // Start at player's location
    auto* player = LEVEL.objects.get(LEVEL.player_id);
    auto obj_pair = create_object(player ? player->position : VecF{});

	// Create implementation
	auto& camera = obj_pair.first;
	camera.impl = std::make_unique<m2::obj::Camera>();

	auto& phy = camera.add_physique();
	phy.post_step = [&camera](MAYBE Physique& phy) {
//		auto* camera_data = dynamic_cast<m2::obj::Camera*>(camera.impl.get());
		auto& player = LEVEL.objects[LEVEL.player_id];
		camera.position = player.position;

		// Call dynamic image loader
		if (LEVEL.type() == Level::Type::SHEET_EDITOR) {
			IF(LEVEL.dynamic_image_loader)->move(GAME.viewport_to_2d_world_rect_m());
			IF(LEVEL.dynamic_grid_lines_loader)->move(GAME.viewport_to_2d_world_rect_m());
			IF(LEVEL.dynamic_sheet_grid_lines_loader)->move(GAME.viewport_to_2d_world_rect_m());
		}

		// Mouse lookahead disabled temporarily
//		if (GAME.level->type() == Level::Type::SINGLE_PLAYER) {
//			// Give an offset to the camera's location based on the position of the mouse
//			m2::VecF offsetWRTScreenCenter = GAME.mousePositionWRTScreenCenter_m.ceil_length(OFFSET_LIMIT);
//			camera_data->offset = camera_data->offset.lerp(offsetWRTScreenCenter, 0.5f * CAMERA_JUMP_RATIO);
//			camera.position = camera.position.lerp(player.position + camera_data->offset, CAMERA_JUMP_RATIO);
//		} else {
//			camera.position = player.position;
//		}

		if (m2g::camera_is_listener) {
			LEVEL.left_listener->position = camera.position;
			LEVEL.right_listener->position = camera.position;
		}
	};

	if (LEVEL.type() == Level::Type::LEVEL_EDITOR || LEVEL.type() == Level::Type::PIXEL_EDITOR || LEVEL.type() == Level::Type::SHEET_EDITOR) {
		// Add graphics
		auto& gfx = camera.add_graphic();
		gfx.on_draw = [&](MAYBE Graphic& gfx) {
			auto* camera_data = dynamic_cast<m2::obj::Camera*>(camera.impl.get());
			if (camera_data->draw_grid_lines || LEVEL.type() == Level::Type::SHEET_EDITOR) {
				//draw_grid_lines(camera.position);
			}
		};
	}

	if (m2g::camera_is_listener) {
		LEVEL.left_listener = SoundListener{.position = LEVEL.player()->position, .direction = PI, .listen_angle = PI_DIV2};
		LEVEL.right_listener = SoundListener{.position = LEVEL.player()->position, .direction = 0.0f, .listen_angle = PI_DIV2};
	}

	LEVEL.camera_id = obj_pair.second;
    return obj_pair.second;
}
