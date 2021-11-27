#include "../Object.h"
#include "../Main.h"

void Camera_postPhysics(ComponentEventListener* el) {
	Object* camera = Pool_GetById(&GAME->objects, el->super.objId);
	Object* player = Pool_GetById(&GAME->objects, GAME->playerId);
	if (camera && player) {
		camera->position = player->position;
	}
}

int ObjectCamera_Init(Object* obj) {
	REFLECT_ERROR(Object_Init(obj, (Vec2F) { 0.0f, 0.0f }, false));

	ComponentEventListener* el = Object_AddEventListener(obj, NULL);
	el->postPhysics = Camera_postPhysics;

	return 0;
}
