#ifndef CAMERA_H
#define CAMERA_H

#include "Object.h"
#include "Error.h"

typedef struct _Camera {
	Object *player;
} Camera;

int CameraInit(Object *obj, Object *player);

#endif
