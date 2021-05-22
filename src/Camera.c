#include "Camera.h"

#define AsObject(object) ((Object*) (object))

static void Camera_postPhysics(Object *obj) {
	obj->pos = AsObject(obj->privData)->pos;
}

int CameraInit(Object *obj, Object *player) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->postPhysics = Camera_postPhysics;
	obj->privData = player;
	return 0;
}

void CameraDeinit(Object *obj) {
	if (obj->privData) {
		free(obj->privData);
	}
	ObjectDeinit(obj);
}
