#include "../Object.h"
#include "../Main.h"

void Camera_postPhysics(ComponentEventListener* el) {
	Level* level = CurrentLevel();
	Object* camera = Bucket_GetById(&level->objects, el->super.objId);
	Object* player = Bucket_GetById(&level->objects, level->playerId);
	if (camera && player) {
		camera->position = player->position;
	}
}

int ObjectCamera_Init(Object* obj) {
	PROPAGATE_ERROR(Object_Init(obj, (Vec2F) { 0.0f, 0.0f }));

	ComponentEventListener* el = Object_AddEventListener(obj, NULL);
	el->postPhysics = Camera_postPhysics;

	return 0;
}
