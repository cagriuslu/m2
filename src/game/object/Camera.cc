#include "m2/game/object/Camera.h"

#include "m2/Game.h"
#include "m2/Object.h"

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

m2::Id m2::obj::CreateCamera() {
	// Start at player's location
	auto* playerPtr = M2_LEVEL.objects.Get(M2_LEVEL.playerId);
	auto it = CreateObject();

	// Create implementation
	auto& camera = *it;
	camera.impl = std::make_unique<m2::obj::Camera>();

	// Position of the camera isn't actually deterministic
	auto& phy = camera.AddPhysique(VecFE{playerPtr ? playerPtr->InferPositionF() : VecF{}});
	phy.postStep = [](MAYBE Physique& phy, const Stopwatch::Duration&) {
		//		auto* camera_data = dynamic_cast<m2::obj::Camera*>(camera.impl.get());
		const auto& player = M2_LEVEL.objects[M2_LEVEL.playerId];
		phy.position = VecFE{player.InferPositionF()};

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
			M2_LEVEL.leftListener->position = static_cast<VecF>(phy.position);
			M2_LEVEL.rightListener->position = static_cast<VecF>(phy.position);
		}
	};

	if (M2G_PROXY.camera_is_listener) {
		M2_LEVEL.leftListener =
		    SoundListener{.position = M2_PLAYER.InferPositionF(), .direction = PI, .listenAngle = PI_DIV2};
		M2_LEVEL.rightListener =
		    SoundListener{.position = M2_PLAYER.InferPositionF(), .direction = 0.0f, .listenAngle = PI_DIV2};
	}

	return M2_LEVEL.cameraId = it.GetId();
}
