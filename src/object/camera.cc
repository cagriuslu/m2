#include <m2/object/camera.h>
#include "m2/object.hh"
#include "m2/Game.hh"

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

static void camera_post_physics(Monitor& el) {
	auto& cam = GAME.objects[el.object_id];
	auto* camera_data = dynamic_cast<m2::object::camera::Data*>(cam.data_new.get());
	auto& player = GAME.objects[GAME.playerId];

	// Give an offset to the camera's location based on the position of the mouse
	m2::vec2f offsetWRTScreenCenter = GAME.mousePositionWRTScreenCenter_m.ceil_length(OFFSET_LIMIT);
	camera_data->offset = camera_data->offset.lerp(offsetWRTScreenCenter, 0.5f * CAMERA_JUMP_RATIO);
	cam.position = cam.position.lerp(player.position + camera_data->offset, CAMERA_JUMP_RATIO);
}

std::pair<m2::object::Object&, ID> m2::object::camera::create() {
    // Start at player's location
    auto* player = GAME.objects.get(GAME.playerId);
    auto obj_pair = object::create(player ? player->position : vec2f{});

    auto& mon = obj_pair.first.add_monitor();
    mon.postPhysics = camera_post_physics;

    obj_pair.first.data_new = std::make_unique<m2::object::camera::Data>();

    GAME.cameraId = obj_pair.second;
    return obj_pair;
}
