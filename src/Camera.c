#include "Camera.h"

#define AsObject(object) ((GameObject*) (object))

static void Camera_postPhysics(GameObject *obj) {
	obj->pos = AsObject(obj->privData)->pos;
}

void Camera_deinit(GameObject* obj) {
	if (obj->privData) {
		free(obj->privData);
	}
}

int CameraInit(GameObject *obj, GameObject *player) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->postPhysics = Camera_postPhysics;
	obj->privData = player;
	obj->deinit = Camera_deinit;
	return 0;
}
