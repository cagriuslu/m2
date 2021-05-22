#include "Camera.h"

typedef struct _CameraData {
	Object *player;
} CameraData;
#define AsCameraData(p) ((CameraData*) (p))

static void Camera_postPhysics(Object *obj) {
	obj->pos = AsCameraData(obj->privData)->player->pos;
}

int CameraInit(Object *obj, Object *player) {
	PROPAGATE_ERROR(ObjectInit(obj));
	CameraData *camData = malloc(sizeof(CameraData));
	if (!camData) {
		return ERR_OUT_OF_MEMORY;
	}
	camData->player = player;
	obj->privData = camData;
	obj->postPhysics = Camera_postPhysics;
	return 0;
}

void CameraDeinit(Object *obj) {
	if (obj->privData) {
		free(obj->privData);
	}
	ObjectDeinit(obj);
}
