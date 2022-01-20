#include "../Object.h"
#include "../Game.h"

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

void Camera_postPhysics(ComponentMonitor* el) {
	Object* camera = Pool_GetById(&GAME->objects, el->super.objId); XASSERT(camera);
	Object* player = Pool_GetById(&GAME->objects, GAME->playerId); XASSERT(player);
	// Give an offset to the camera's location based on the position of the mouse
	Vec2F offsetWRTScreenCenter = Vec2F_CeilLength(GAME->mousePositionWRTScreenCenter_m, OFFSET_LIMIT);
	camera->ex->value.camera.offset = Vec2F_Lerp(camera->ex->value.camera.offset, offsetWRTScreenCenter, 0.5f * CAMERA_JUMP_RATIO);
	camera->position = Vec2F_Lerp(camera->position, Vec2F_Add(player->position, camera->ex->value.camera.offset), CAMERA_JUMP_RATIO);
}

int ObjectCamera_Init(Object* obj) {
	XERR_REFLECT(Object_Init(obj, (Vec2F) {0.0f, 0.0f }, true));

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->postPhysics = Camera_postPhysics;

	return 0;
}
