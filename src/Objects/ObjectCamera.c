#include "../Object.h"
#include "../Game.h"

void Camera_postPhysics(ComponentMonitor* el) {
	Object* camera = Pool_GetById(&GAME->objects, el->super.objId); XASSERT(camera);
	Object* player = Pool_GetById(&GAME->objects, GAME->playerId); XASSERT(player);
	camera->position = player->position;
}

int ObjectCamera_Init(Object* obj) {
	XERR_REFLECT(Object_Init(obj, (Vec2F) {0.0f, 0.0f }, false));

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->postPhysics = Camera_postPhysics;

	return 0;
}
