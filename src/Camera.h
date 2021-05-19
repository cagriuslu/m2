#ifndef CAMERA_H
#define CAMERA_H

#include "Object.h"
#include "Error.h"

typedef struct _CameraData {
	Object *player;
} CameraData;

int CameraInit(Object *obj, Object *player);
void CameraDeinit(Object *obj);

#endif
