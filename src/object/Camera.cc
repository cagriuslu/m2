#include <m2/object/Camera.h>
#include "m2/Object.h"
#include "m2/Game.h"

#include <m2/object/Line.h>

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

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

		// Move dynamic loaders if they exist
		IF(LEVEL.dynamic_image_loader)->move(GAME.viewport_to_2d_world_rect_m());
		IF(LEVEL.dynamic_grid_lines_loader)->move(GAME.viewport_to_2d_world_rect_m());
		IF(LEVEL.dynamic_sheet_grid_lines_loader)->move(GAME.viewport_to_2d_world_rect_m());

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

	if (m2g::camera_is_listener) {
		LEVEL.left_listener = SoundListener{.position = LEVEL.player()->position, .direction = PI, .listen_angle = PI_DIV2};
		LEVEL.right_listener = SoundListener{.position = LEVEL.player()->position, .direction = 0.0f, .listen_angle = PI_DIV2};
	}

	LEVEL.camera_id = obj_pair.second;
    return obj_pair.second;
}
