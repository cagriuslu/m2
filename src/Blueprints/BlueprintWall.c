#include "../Blueprint.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Box2DUtils.h"

void Wall_deinit(Object* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int BlueprintWallInit(Object* obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->pos = position;
	obj->txSrc = (SDL_Rect){ 8 * TILE_WIDTH, 0, TILE_WIDTH, 2 * TILE_WIDTH };
	obj->txOffset = (Vec2F){ 0.0, -12.0 };
	obj->body = Box2DUtilsCreateStaticBox(position, ALLOW_SLEEP, NOT_SENSOR, (Vec2F) { 0.875, 0.875 });
	obj->deinit = Wall_deinit;
	return 0;
}
