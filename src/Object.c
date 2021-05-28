#include "Object.h"
#include <string.h>

int ObjectInit(Object *obj) {
	memset(obj, 0, sizeof(Object));
	return 0;
}

void ObjectDeinit(Object *obj) {
	if (obj->deinit) {
		obj->deinit(obj);
	}
}
