#include "m2/game/object/Camera.h"

#include "m2/Game.h"
#include "m2/Object.h"
#include "m2/game/object/Line.h"

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

m2::Id m2::obj::create_camera() {
	// Start at player's location
	auto* player = M2_LEVEL.objects.get(M2_LEVEL.playerId);
	auto it = create_object(player ? player->position : VecF{});

	// Create implementation
	auto& camera = *it;
	camera.impl = std::make_unique<m2::obj::Camera>();

	auto& phy = camera.add_physique();
	phy.post_step = [&camera](MAYBE Physique& phy) {
		//		auto* camera_data = dynamic_cast<m2::obj::Camera*>(camera.impl.get());
		auto& player = M2_LEVEL.objects[M2_LEVEL.playerId];
		camera.position = player.position;

		// Move dynamic loaders if they exist
		std::visit(
		    overloaded{
		        [](sheet_editor::State& ss) {
			        if (auto* dil = ss.dynamic_image_loader()) {
				        dil->move(ViewportM());
			        }
		        },
		        [](bulk_sheet_editor::State& ss) {
			        if (auto* dil = ss.dynamic_sprite_sheet_loader()) {
				        dil->move(ViewportM());
			        }
		        },
		        DEFAULT_OVERLOAD},
		    M2_LEVEL.stateVariant);

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
			M2_LEVEL.leftListener->position = camera.position;
			M2_LEVEL.rightListener->position = camera.position;
		}
	};

	if (M2G_PROXY.camera_is_listener) {
		M2_LEVEL.leftListener =
		    SoundListener{.position = M2_PLAYER.position, .direction = PI, .listen_angle = PI_DIV2};
		M2_LEVEL.rightListener =
		    SoundListener{.position = M2_PLAYER.position, .direction = 0.0f, .listen_angle = PI_DIV2};
	}

	return M2_LEVEL.cameraId = it.id();
}
