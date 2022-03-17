#include <m2/Object.hh>
#include <m2/Game.hh>

// TODO this depends on the physics loop freq rather than time
#define CAMERA_JUMP_RATIO (4.0f / 50.0f)
#define OFFSET_LIMIT (1.0f)

typedef struct {
	Vec2F offset;
} CameraData;
#define AS_CAMERADATA(ptr) ((CameraData*)(ptr))

void Camera_postPhysics(ComponentMonitor* el) {
	Object* camera = static_cast<Object *>(Pool_GetById(&GAME->objects, el->super.objId)); M2ASSERT(camera);
	Object* player = static_cast<Object *>(Pool_GetById(&GAME->objects, GAME->playerId)); M2ASSERT(player);
	// Give an offset to the camera's location based on the position of the mouse
	Vec2F offsetWRTScreenCenter = Vec2F_CeilLength(GAME->mousePositionWRTScreenCenter_m, OFFSET_LIMIT);
	AS_CAMERADATA(camera->data)->offset = Vec2F_Lerp(AS_CAMERADATA(camera->data)->offset, offsetWRTScreenCenter, 0.5f * CAMERA_JUMP_RATIO);
	camera->position = Vec2F_Lerp(camera->position, Vec2F_Add(player->position, AS_CAMERADATA(camera->data)->offset), CAMERA_JUMP_RATIO);
}

M2Err ObjectCamera_Init(Object* obj) {
	M2ERR_REFLECT(Object_Init(obj, VEC2F_ZERO));
	obj->data = calloc(1, sizeof(CameraData)); M2ASSERT(obj->data);

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->postPhysics = Camera_postPhysics;

	return M2OK;
}
