#include "../Object.h"
#include "../Main.h"

void Camera_postPhysics(EventListenerComponent* el) {
	Level* level = CurrentLevel();
	Object* camera = BucketGetById(&level->objects, el->super.objId);
	Object* player = BucketGetById(&level->objects, level->playerId);
	if (camera && player) {
		camera->position = player->position;
	}
}

int ObjectCameraInit(Object* obj) {
	PROPAGATE_ERROR(ObjectInit(obj, (Vec2F) { 0.0f, 0.0f }));

	EventListenerComponent* el = ObjectAddAndInitEventListener(obj, NULL);
	el->postPhysics = Camera_postPhysics;

	return 0;
}
