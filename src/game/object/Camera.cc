#include "m2/game/object/Camera.h"

#include "m2/Game.h"
#include "m2/Object.h"
#include "m2/game/object/Line.h"

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

m2::Id m2::obj::create_camera() {
	// Start at player's location
	auto* player = M2_LEVEL.objects.get(M2_LEVEL.player_id);
	auto obj_pair = create_object(player ? player->position : VecF{});

	// Create implementation
	auto& camera = obj_pair.first;
	camera.impl = std::make_unique<m2::obj::Camera>();

	auto& phy = camera.add_physique();
	phy.post_step = [&camera](MAYBE Physique& phy) {
		//		auto* camera_data = dynamic_cast<m2::obj::Camera*>(camera.impl.get());
		auto& player = M2_LEVEL.objects[M2_LEVEL.player_id];
		camera.position = player.position;

		// Move dynamic loaders if they exist
		std::visit(
		    overloaded{
		        [](sedit::State& ss) {
			        if (auto* dil = ss.dynamic_image_loader()) {
				        dil->move(M2_GAME.viewport_to_2d_world_rect_m());
			        }
		        },
		        [](bsedit::State& ss) {
			        if (auto* dil = ss.dynamic_sprite_sheet_loader()) {
				        dil->move(M2_GAME.viewport_to_2d_world_rect_m());
			        }
		        },
		        DEFAULT_OVERLOAD},
		    M2_LEVEL.type_state);
		IF(M2_LEVEL.dynamic_grid_lines_loader)->move(M2_GAME.viewport_to_2d_world_rect_m());
		IF(M2_LEVEL.dynamic_sheet_grid_lines_loader)->move(M2_GAME.viewport_to_2d_world_rect_m());

		// Mouse lookahead disabled temporarily
		//		if (M2_GAME.level->type() == Level::Type::SINGLE_PLAYER) {
		//			// Give an offset to the camera's location based on the position of the mouse
		//			m2::VecF offsetWRTScreenCenter = M2_GAME.mousePositionWRTScreenCenter_m.ceil_length(OFFSET_LIMIT);
		//			camera_data->offset = camera_data->offset.lerp(offsetWRTScreenCenter, 0.5f * CAMERA_JUMP_RATIO);
		//			camera.position = camera.position.lerp(player.position + camera_data->offset, CAMERA_JUMP_RATIO);
		//		} else {
		//			camera.position = player.position;
		//		}

		if (M2G_PROXY.camera_is_listener) {
			M2_LEVEL.left_listener->position = camera.position;
			M2_LEVEL.right_listener->position = camera.position;
		}
	};

	if (M2G_PROXY.camera_is_listener) {
		M2_LEVEL.left_listener =
		    SoundListener{.position = M2_PLAYER.position, .direction = PI, .listen_angle = PI_DIV2};
		M2_LEVEL.right_listener =
		    SoundListener{.position = M2_PLAYER.position, .direction = 0.0f, .listen_angle = PI_DIV2};
	}

	M2_LEVEL.camera_id = obj_pair.second;
	return obj_pair.second;
}
