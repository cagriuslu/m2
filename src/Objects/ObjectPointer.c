#include "m2/Object.h"
#include "m2/Game.h"
#include "m2/Def.h"

void Pointer_preGraphics(ComponentMonitor* mon) {
	Object* obj = Pool_GetById(&GAME->objects, mon->super.objId); M2ASSERT(obj);
	obj->position = GAME->mousePositionInWorld_m;
}

int ObjectPointer_Init(Object* obj) {
	M2ERR_REFLECT(Object_Init(obj, (Vec2F) {0.0f, 0.0f}));

	ComponentMonitor* mon = Object_AddMonitor(obj);
	mon->preGraphics = Pointer_preGraphics;

	ComponentLight* lig = Object_AddLight(obj);
	lig->radius_m = 3.0f;

	return M2OK;
}
