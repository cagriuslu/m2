#include "m2/Object.hh"
#include "m2/Game.hh"
#include "m2/Def.hh"

void Pointer_preGraphics(ComponentMonitor* mon) {
    auto* obj = GAME.objects.get(mon->super.objId);
	obj->position = GAME.mousePositionInWorld_m;
}

int ObjectPointer_Init(Object* obj) {
	M2ERR_REFLECT(Object_Init(obj, {}));

	ComponentMonitor* mon = Object_AddMonitor(obj);
	mon->preGraphics = Pointer_preGraphics;

	ComponentLight* lig = Object_AddLight(obj);
	lig->radius_m = 3.0f;

	return M2OK;
}
