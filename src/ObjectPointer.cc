#include <m2/object/Object.hh>
#include "m2/Game.hh"
#include "m2/Def.hh"

void Pointer_preGraphics(Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	obj.position = GAME.mousePositionInWorld_m;
}

int ObjectPointer_Init(m2::object::Object* obj) {
	*obj = m2::object::Object{m2::vec2f{}};

	auto& mon = obj->add_monitor();
	mon.preGraphics = Pointer_preGraphics;

	auto& light = obj->add_light();
	light.radius_m = 3.0f;

	return M2OK;
}
