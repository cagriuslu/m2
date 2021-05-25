#include "God.h"
#include "Error.h"

int GodInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	return 0;
}

void GodDeinit(Object *obj) {
	ObjectDeinit(obj);
}