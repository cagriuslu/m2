#include <m2/object/Camera.h>
#include "m2/Object.h"
#include "m2/Game.hh"

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

std::pair<m2::Object&, ID> m2::obj::create_camera() {
    // Start at player's location
    auto* player = GAME.objects.get(GAME.playerId);
    auto obj_pair = create_object(player ? player->position : Vec2f{});
	auto& camera = obj_pair.first;
	auto camera_id = obj_pair.second;

    auto& mon = camera.add_monitor();

	camera.impl = std::make_unique<m2::obj::Camera>();

	mon.post_phy = [&]([[maybe_unused]] m2::comp::Monitor& el) {
		auto* camera_data = dynamic_cast<m2::obj::Camera*>(camera.impl.get());
		auto& player = GAME.objects[GAME.playerId];

		// Give an offset to the camera's location based on the position of the mouse
		m2::Vec2f offsetWRTScreenCenter = GAME.mousePositionWRTScreenCenter_m.ceil_length(OFFSET_LIMIT);
		camera_data->offset = camera_data->offset.lerp(offsetWRTScreenCenter, 0.5f * CAMERA_JUMP_RATIO);
		camera.position = camera.position.lerp(player.position + camera_data->offset, CAMERA_JUMP_RATIO);
	};

    GAME.cameraId = camera_id;
    return obj_pair;
}
