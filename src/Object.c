#include "Object.h"
#include <string.h>

static void Object_ovrdGraphics(Object *obj, SDL_Renderer *renderer) {

}

int ObjectInit(Object *obj) {
	memset(obj, 0, sizeof(Object));
	obj->grav = 9.80665;
	obj->txScaleW = 1.0;
	obj->txScaleH = 1.0;
	obj->ovrdGraphics = Object_ovrdGraphics;
	return X_OK;
}

void ObjectDeinit(Object *obj) {
	
}
