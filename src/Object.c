#include "Object.h"

int ObjectInit(Object *obj) {
	obj->pos = (Point) {0, 0, 0};

	return X_OK;
}
