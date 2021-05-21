#include "Object.h"
#include <string.h>

static void Object_ovrdGraphics(Object *obj) {

}

int ObjectInit(Object *obj) {
	memset(obj, 0, sizeof(Object));
	obj->txScaleW = 1.0;
	obj->txScaleH = 1.0;
	obj->ovrdGraphics = Object_ovrdGraphics;
	return 0;
}

void ObjectDeinit(Object *obj) {
	
}
