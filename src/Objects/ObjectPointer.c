#include "../Object.h"
#include "../Game.h"
#include "../Def.h"

void Pointer_preGraphics(ComponentMonitor* mon) {
	Object* obj = Pool_GetById(&GAME->objects, mon->super.objId); XASSERT(obj);
	obj->position = GAME->mousePositionInWorld;
}

int ObjectPointer_Init(Object* obj) {
	XERR_REFLECT(Object_Init(obj, (Vec2F) {0.0f, 0.0f}, false));

	ComponentMonitor* mon = Object_AddMonitor(obj);
	mon->preGraphics = Pointer_preGraphics;

	ComponentLight* lig = Object_AddLight(obj);
	lig->radius_m = 3.0f;

	return XOK;
}
