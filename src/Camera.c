#include "Camera.h"

#define AsCameraData(p) ((CameraData*) (p))

static void Camera_postPhysics(Object *obj) {
	obj->pos = AsCameraData(obj->privData)->player->pos;
}

int CameraInit(Object *obj, Object *player) {
	int res = ObjectInit(obj);
	if (res != X_OK) {
		return res;
	}
	CameraData *camData = malloc(sizeof(CameraData));
	if (!camData) {
		return X_OUT_OF_MEMORY;
	}
	camData->player = player;
	obj->privData = camData;
	obj->postPhysics = Camera_postPhysics;
	return X_OK;
}

void CameraDeinit(Object *obj) {
	if (obj->privData) {
		free(obj->privData);
	}
	ObjectDeinit(obj);
}
