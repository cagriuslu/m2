#include "Camera.h"
#include <m2/object/Object.hh>
#include "m2/Game.hh"

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

m2::object::CameraData::CameraData() : offset() {}

void Camera_postPhysics(ComponentMonitor* el) {
	Object* camera = GAME.objects.get(el->super.objId);
	auto* camera_data = dynamic_cast<m2::object::CameraData*>(camera->data_new.get());
	Object* player = GAME.objects.get(GAME.playerId);

	// Give an offset to the camera's location based on the position of the mouse
	m2::vec2f offsetWRTScreenCenter = GAME.mousePositionWRTScreenCenter_m.ceil_length(OFFSET_LIMIT);
	camera_data->offset = camera_data->offset.lerp(offsetWRTScreenCenter, 0.5f * CAMERA_JUMP_RATIO);
	camera->position = camera->position.lerp(player->position + camera_data->offset, CAMERA_JUMP_RATIO);
}

M2Err ObjectCamera_Init(Object* obj) {
	// Start at player's location
	Object *player = GAME.objects.get(GAME.playerId);
	M2ERR_REFLECT(Object_Init(obj, player->position));

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->postPhysics = Camera_postPhysics;

	obj->data_new = std::make_unique<m2::object::CameraData>();
	return M2OK;
}
