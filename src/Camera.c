#include "Camera.h"

#define AsObject(object) ((Object*) (object))

static void Camera_postPhysics(Object *obj) {
	obj->pos = AsObject(obj->privData)->pos;
}

void Camera_deinit(Object* obj) {
	if (obj->privData) {
		free(obj->privData);
	}
}

int CameraInit(Object *obj, Object *player) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->postPhysics = Camera_postPhysics;
	obj->privData = player;
	obj->deinit = Camera_deinit;
	return 0;
}
