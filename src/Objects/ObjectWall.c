#include "../Object.h"
#include "../Game.h"
#include "../Box2D.h"
#include "../Box2DUtils.h"

int ObjectWall_Init(Object* obj, Vec2F position) {
	M2ERR_REFLECT(Object_Init(obj, position, false));
	
	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateStaticBox(Pool_GetId(&GAME->physics, phy), position, CATEGORY_STATIC_OBJECT, ((Vec2F) {0.875, 0.125 }));

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	// TODO

	return 0;
}
